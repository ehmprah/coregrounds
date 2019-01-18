(function ($) {

  //vertical centering
  $.fn.verticalCenter = function() {
    if(this.is(":visible")) {
      var th = this.outerHeight();
      var wh = $(window).height();
      var mt = 0;
      if(wh > th) {
        mt = (wh - th) / 2;
      }
      this.css('margin-top',mt);
    }
    return this;
  };

  $.fn.charCount = function(limit) {
    var input = this.find('textarea').after('<div class="char-counter grey"></div>');
    var counter = this.find('.char-counter');
    counter.html(input.val().length + ' / ' + limit + ' characters');
    input.keyup(function(){
      if ($(this).val().length > limit) {
        counter.css('color','red');
      } else {
        counter.css('color','#888');
      }
      counter.html(input.val().length + ' / ' + limit + ' characters');
    });
  };

  if(typeof Drupal !== 'undefined') {

    Drupal.behaviors.dimissMsgs = {
      attach: function(context) {
        $("a.dismiss").one('click',function(){
          if($(this).data('coins')) {
            $('#credits-current').kaching().html(parseInt($('#credits-current').html(),10) + $(this).data('coins')).animate({fontSize:20},200,"linear",function(){ $(this).animate({fontSize:18},200); });
          }
          $(this).parents('.messages').fadeOut(400);
          return false;
        });
      }
    }

    Drupal.behaviors.charCounter = {
      attach: function(context) {
        //character count for content form
        if ($('.field-name-field-meta-description').length) {
          $('.field-name-field-meta-description').charCount(160);
        }
      }
    }

    Drupal.behaviors.xpandMatchHistory = {
      attach: function(context) {
        $('.match.expanded').find('.expanded-stats').show();
        $('.match').once('xpand').on('click', function () {
          $(this).toggleClass('expanded').find('.expanded-stats').slideToggle(400);
        });
      }
    }

    Drupal.behaviors.AccountSettings = {
      attach: function(context) {
        if($("#emblem").length) {
          $("#emblem").imagepicker();
        }
        var checkboxes = $('.form-item-emotes input[type="checkbox"]');
        checkboxes.filter(':not(:checked)').prop('disabled', checkboxes.filter(':checked').length >= 7);
        checkboxes.change(function(){
          checkboxes.filter(':not(:checked)').prop('disabled', checkboxes.filter(':checked').length >= 7);
        });
      }
    }

    Drupal.behaviors.vCenter = {
      attach: function(context) {
        $('.vcenter').each(function(){
            $(this).verticalCenter();
        });

        $(window).resize(function(){
            $('.vcenter').each(function(){
                $(this).verticalCenter();
            });
        });
      }
    }

    Drupal.behaviors.autoplayVideos = {
      attach: function(context) {
        $('a.load-video').on('click', function () {
          if(typeof $(this).data('video') !== 'undefined') {
            var iframe = document.createElement("iframe");
            iframe.setAttribute("src","https://www.youtube-nocookie.com/embed/" + $(this).data('video') + "?autoplay=1");
            iframe.setAttribute("style","position:absolute;top:0;left:0;width:100%;height:100%;border:0;");
            iframe.setAttribute("allowfullscreen","");
            var wrap = $('<div class="video"></div>');
            $(wrap).append(iframe);
            $(this).replaceWith(wrap);
            return false;
          }
          if(typeof $(this).data('playlist') !== 'undefined') {
            var iframe = document.createElement("iframe");
            iframe.setAttribute("src","https://www.youtube-nocookie.com/embed/videoseries?list=" + $(this).data('playlist') + "&autoplay=1");
            iframe.setAttribute("style","position:absolute;top:0;left:0;width:100%;height:100%;border:0;");
            iframe.setAttribute("allowfullscreen","");
            var wrap = $('<div class="video"></div>');
            $(wrap).append(iframe);
            $(this).replaceWith(wrap);
            return false;
          }
        });
      }
    }
  }

  $(document).ready(function(){
    //slider
    if($('.slider').length) {
      $(".slider").flexslider({
        animation: "slide",
        selector: ".slides > .slide",
        slideshow: false,
      });
    }
  });

  $(document).ready(function(){
    if (("standalone" in window.navigator) && window.navigator.standalone) {
      $('a').on('click', function(e){
        e.preventDefault();
        var new_location = $(this).attr('href');
        if (new_location != undefined && new_location.substr(0, 1) != '#' && $(this).attr('data-method') == undefined){
          window.location = new_location;
        }
      });
    }
  });

})(jQuery);
