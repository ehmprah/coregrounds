/**
 * @file Analytics.js
 * Track app usage via Google Analytics
 *
 * @see https://github.com/Daaru00/nwjs-analytics
 * @see https://developers.google.com/analytics/devguides/collection/analyticsjs/
 *
 * Event tracking:
 * Analytics.event(eventCategory, eventAction, eventLabel, eventValue);
 * Analytics.event(Video, play, 'Fall Campaign', 42);
 * ONLY eventCategory and eventAction are required!
 *
 * TODO rewrite to clean ES6
 * TODO events param 3 and 4 are NOT required!
 * TODO ADD TRACKING FOR: join queue
 * TODO ADD TRACKING FOR: leave queue
 * TODO ADD TRACKING FOR: ecommerce purchase
 * TODO ADD TRACKING FOR: spend stars
 * TODO ADD TRACKING FOR: start tutorial
 * TODO ADD TRACKING FOR: GOAL for completing tutorial
 */
import Event from './Event';
import Session from './Session/Session';
import { md5 } from './Util/Mixed';
import { logToConsole } from './Console/Console';
import Config from './Config/Config';

function uuidv4() {
  return ([1e7]+-1e3+-4e3+-8e3+-1e11).replace(/[018]/g, c =>
    (c ^ crypto.getRandomValues(new Uint8Array(1))[0] & 15 >> c / 4).toString(16)
  )
}

Event.on('app loaded', function() {
  // Try to get client id from storage
  let uuid = window.localStorage.getItem('cgs-uuid');
  if (!uuid) {
    // Generate one if we haven't so far
    uuid = uuidv4();
    window.localStorage.setItem('cgs-uuid', uuid);
  }
  // Pass the uuid on to the module
  Analytics.clientID = uuid;
});

Event.on('session ready', function() {
  let account = Session.getAccount();
  // Create a PII safe hash unique for this user
  Analytics.userID = md5(account.uid + account.name);
});

const Analytics = {
    apiVersion: '1',
    trackID: 'UA-60150088-3',
    clientID: null,
    userID: null,
    appName: 'coregrounds',
    appVersion: Config.VERSION,
    debug: false,
    performanceTracking: false,
    errorTracking: true,
    userLanguage: 'en',
    currency: 'EUR',
    lastScreenName: '',

    sendRequest: function(data, callback){
        if (!this.clientID || this.clientID == null)
            this.clientID = this.generateClientID();

        if (!this.userID || this.userID == null)
            this.userID = this.generateClientID();

        var postData = "v="+this.apiVersion
                        +"&aip=1" // Anonymize IP
                        +"&tid="+this.trackID
                        +"&cid="+this.clientID
                        +"&uid="+this.userID
                        +"&an="+this.appName
                        +"&av="+this.appVersion
                        +"&sr="+this.getScreenResolution()
                        +"&vp="+this.getViewportSize()
                        +"&sd="+this.getColorDept()
                        +"&ul="+this.userLanguage
                        +"&ua="+this.getUserAgent()
                        +"&ds=app";

        Object.keys(data).forEach(function(key) {
            var val = data[key];
            if (val !== undefined)
                postData += "&"+key+"="+val;
        });

        var http = new XMLHttpRequest();
        var url = "https://www.google-analytics.com";
        if (!this.debug)
            url += "/collect";
        else
            url += "/debug/collect";

        http.open("POST", url, true);

        http.setRequestHeader("Content-type", "application/x-www-form-urlencoded");

        http.onreadystatechange = function() {
            if (Analytics.debug)
                console.log(http.response);

            if (http.readyState == 4 && http.status == 200) {
                if (callback)
                    callback(true);
            }
            else
            {
                if (callback)
                    callback(false);
            }
        }
        http.send(postData);
    },
    generateClientID: function()
    {
        var id = "";
        var possibilities = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
        for( var i=0; i < 5; i++ )
            id += possibilities.charAt(Math.floor(Math.random() * possibilities.length));
        return id;
    },
    getScreenResolution: function(){
        return screen.width+"x"+screen.height;
    },
    getColorDept: function(){
        return screen.colorDepth+"-bits";
    },
    getUserAgent: function(){
        return navigator.userAgent;
    },
    getViewportSize: function(){
        return window.screen.availWidth+"x"+window.screen.availHeight;
    },

    /*
     * Measurement Protocol
     * [https://developers.google.com/analytics/devguides/collection/protocol/v1/devguide]
     */

    screenView: function(screename){
        var data = {
			't' : 'screenview',
			'cd' : screename
		}
		this.sendRequest(data);
        this.lastScreenName = screename;
    },
    event: function(category, action, label, value){
        var data = {
			't' : 'event',
			'ec' : category,
			'ea' : action,
			'el' : label,
			'ev' : value,
            'cd' : this.lastScreenName,
		}
		this.sendRequest(data);
    },
    exception: function(msg, fatal){
        var data = {
			't' : 'exception',
			'exd' : msg,
			'exf' : fatal || 0
		}
		this.sendRequest(data);
    },
    timing: function(category, variable, time, label){

        var data = {
			't' : 'timing',
			'utc' : category,
			'utv' : variable,
			'utt' : time,
			'utl' : label,
		}
		this.sendRequest(data);
    },
    ecommerce:{
        transactionID: false,
        generateTransactionID: function()
        {
            var id = "";
            var possibilities = "0123456789";
            for( var i=0; i < 5; i++ )
                id += possibilities.charAt(Math.floor(Math.random() * possibilities.length));
            return id;
        },
        transaction: function(total, items){
            var t_id = "";
            if (!this.ecommerce.transactionID)
                t_id = this.ecommerce.generateTransactionID();
            else
                t_id = this.ecommerce.transactionID;

            var data = {
                't' : 'transaction',
                'ti' : t_id,
                'tr' : total,
                'cu' : this.currency,
            }
            this.sendRequest(data);

            items.forEach(function(item){
                var data = {
                    't' : 'item',
                    'ti' : t_id,
                    'in' : item.name,
                    'ip' : item.price,
                    'iq' : item.qty,
                    'ic' : item.id,
                    'cu' : this.currency
                }
                this.sendRequest(data);
            })
        }
    },
    custom: function(data){
        this.sendRequest(data);
    }
}

/*
 * Performance Tracking
 */
window.addEventListener("load", function() {
  if (Analytics.performanceTracking) {
    setTimeout(function() {
      var timing = window.performance.timing;
      var userTime = timing.loadEventEnd - timing.navigationStart;
      Analytics.timing('performance', 'pageload', userTime);
    }, 0);
  }
}, false);

/*
 * Error Reporting
 */
window.onerror = function (msg, url, lineNo, columnNo, error) {
  // Log to dev console as well
  logToConsole(`Message: ${msg}, Error: ${JSON.stringify(error)}`, 'error');
  if (Analytics.errorTracking) {
    var message = [
        'Message: ' + msg,
        'Line: ' + lineNo,
        'Column: ' + columnNo,
        'Error object: ' + JSON.stringify(error)
    ].join(' - ');
    setTimeout(function() {
      Analytics.exception(message.toString());
    }, 0);
  }
  return false;
};

export default Analytics;
