/**
 * @file Util.js
 * Collection of helper functions
 *
 * @todo separate this file into different categories like dom, geometry, ...
 */
import Event from './Event';



// GEOMETRY

export function pointIsOnTile(x, y, a) {
  let b = pointToTile(x, y);
  return a[0] == b[0] && a[1] == b[1];
}

export function pointsDistance(ax, ay, bx, by) {
  return Math.sqrt((ax-bx)*(ax-bx) + (ay-by)*(ay-by));
}

export function pointToTile(x, y) {
  var tile = [];
  tile.push(x / 128 | 0);
  tile.push(y / 128 | 0);
  return tile;
}


export function validateEmail(email) {
  var re = /^(([^<>()\[\]\\.,;:\s@"]+(\.[^<>()\[\]\\.,;:\s@"]+)*)|(".+"))@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\])|(([a-zA-Z\-0-9]+\.)+[a-zA-Z]{2,}))$/;
  return re.test(email);
}





// DOM / HTML



export function resize() { // TODO this belongs in a module

  // Resize the game area
  var gameArea = document.getElementById('wrapper');
  var widthToHeight = 16/9;
  var newWidth = window.innerWidth;
  var newHeight = window.innerHeight;
  var newWidthToHeight = newWidth / newHeight;

  if (newWidthToHeight > widthToHeight) {
    newWidth = newHeight * widthToHeight;
    gameArea.style.height = newHeight + 'px';
    gameArea.style.width = newWidth + 'px';
  } else {
    newHeight = newWidth / widthToHeight;
    gameArea.style.width = newWidth + 'px';
    gameArea.style.height = newHeight + 'px';
  }

  gameArea.style.marginTop = (-newHeight / 2) + 'px';
  gameArea.style.marginLeft = (-newWidth / 2) + 'px';

  // Resize the ambient background canvases
  for (let item of document.querySelectorAll('.ambient')) {
    item.style.height = window.innerHeight + 'px';
    item.style.marginLeft = (-1920 + window.innerWidth)/2 + 'px';
  }

  Event.fire('resize');
}

export function throttle( fn, threshhold, scope ) {
  threshhold || ( threshhold = 250 );
  var last, deferTimer;
  return function () {
    var context = scope || this;

    var now = +new Date,
        args = arguments;
    if ( last && now < last + threshhold ) {
      // hold on to it
      clearTimeout( deferTimer );
      deferTimer = setTimeout( function () {
        last = now;
        fn.apply( context, args );
      }, threshhold );
    } else {
      last = now;
      fn.apply( context, args );
    }
  }
}

export function jitter(range) {
  return Math.random() * range * 2 - range;
}

export function pad(string, length, pad_string) {
  var s = string + "";
  while (s.length < length) s = pad_string + s;
  return s;
}

export function formatTime(seconds) {
  return pad(parseInt(seconds / 60), 2, '0') + ':' +
         pad(parseInt(seconds % 60), 2, '0');
}

export function formatSeconds(time) {
  // Singular
  if (time == 1) return '1 second left';
  // Plural
  return `${time} seconds left`;
}



export function removeHTML(element) {
  while (element.firstChild) {
    element.removeChild(element.firstChild);
  }
}

export function adoptHTML(string, element = false) {
  let template = document.createElement('template');
  template.innerHTML = string;

  if (element) {
    element.appendChild(template.content);
  } else {
    document.body.appendChild(template.content);
  }
}

export function submitHandler(id, validate, submit) {
  // Attach click handler to button
  let btn = document.getElementById(id);
  btn.addEventListener('click', function(e) {
    e.preventDefault();
    if (validate === false || validate()) {
      btn.classList.add('waiting');
      submit();
    }
  });
}

export function buildGrid() {
  let grid = [[]];
  for(var y = 0; y <= 6; y++) {
    grid[y] = [];
    for(var x = 0; x <= 14; x++) {
      grid[y][x] = 0;
    }
  }
  return grid;
}

export function randomInt(max = 1, min = 0) {
  return Math.round(Math.random() * max) + min;
}

export function randomSignedInt(max = 1, min = 0) {
  if (max < min) {
    return Math.round(Math.random() * (min - max)) + max;
  } else {
    return Math.round(Math.random() * (max - min)) + min;
  }
}

export function randomNumber(max = 1, min = 0) {
  return Math.random() * (max - min) + min;
}

/**
 * Builds a timestamp unique per day
 */
export function todayString() {
  return new Date().toJSON().slice(0, 10);
}

/**
 * Returns a random value from a given array
 */
export function arrayRandom(array) {
  return array[Math.floor(Math.random() * array.length)];
}

/**
 * Returns a string with the first letter capitalized
 */
export function capitalizeFirst(string) {
  return string.charAt(0).toUpperCase() + string.slice(1);
}

/**
 * Format a date object into a full date string
 */
export function formatDate(date) {
  var monthNames = [
    "January", "February", "March", "April", "May", "June",
    "July", "August", "September", "October", "November", "December"
  ];

  var day = date.getDate();
  var monthIndex = date.getMonth();
  var year = date.getFullYear();

  return day + ' ' + monthNames[monthIndex] + ' ' + year;
}

export function formatMatchType(type) {
  type = parseInt(type, 10);
  if (type == 0) return 'Bot';
  if (type == 1) return 'Casual';
  if (type == 2) return 'Ranked';
  if (type == 3) return 'Private';
  if (type == 4) return 'Random';
  return false;
}

export function formatSecondDuration(input) {
  var sec_num = parseInt(input, 10);
  var hours   = Math.floor(sec_num / 3600);
  var minutes = Math.floor((sec_num - (hours * 3600)) / 60);
  var seconds = sec_num - (hours * 3600) - (minutes * 60);

  if (hours   < 10) {hours   = "0"+hours;}
  if (minutes < 10) {minutes = "0"+minutes;}
  if (seconds < 10) {seconds = "0"+seconds;}
  return hours+':'+minutes+':'+seconds;
}

/**
 * Format a date object into a full date string
 */
export function addThousandSeparator(number) {
  return number.toString().replace(/\B(?=(\d{3})+(?!\d))/g, ",");
}
