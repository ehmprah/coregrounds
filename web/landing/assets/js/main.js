function itemClicked(sender,id) {
    $('.unit-image').removeClass('selected')
    $(sender).addClass('selected')
    id = id.toUpperCase();
    $('.collapsable').hide(200);
    $('#' + id).show(200);
}

$(document).ready(function() {
    $.ajax({
      url: "assets/server_config.json",
      dataType: "json",
      success: function(config_json) {
        // First we build the factories hash
        var factories = {};
        for (var index in config_json.factories) {
            var curr_fact = config_json.factories[index];
            factories[curr_fact.id] = curr_fact;
        }
          
        // Build towers element
        var towers = config_json.towers;
        for (var index in towers) {
            var tower = towers[index];
            var factory = factories[tower.id]
            var tower_list_template = '<img onclick="itemClicked(this, \'{0}\')" class="unit-image tower-image" src="{1}"/>'.replace('{0}', tower.name.trim()).replace('{1}', 'assets/units/' + tower.id + '.svg');
            $(tower_list_template).appendTo('#tower-images')
            var tower_desc_template = "<div class='collapsable' id='{0}'>\
                                    <span class='tower-name'>{0}</span>\
                                    <p class='tower-desc'>\
                                    {1} <br/>\
                                    {2} <br/>\
                                    <span class='difficulty '>Difficulty: <span class={3}>{3}</span></span>\
                                    </p>\
                                </div>".replace(/\{0\}/g, tower.name.toUpperCase()).replace('{1}', factories[tower.id].description).replace('{2}', factories[tower.id].meta).replace(/\{3\}/g, factory.difficulty);
            $(tower_desc_template).appendTo('#tower-descs')
        }
        minions = config_json.minions;
        for (var index in config_json.minions) {
            var minion = minions[index];
            var factory = factories[minion.id]
            // If this minion has no factory, we skip it
            if (!factory) {
                continue;
            }
            var minion_list_template = '<img onclick="itemClicked(this, \'{0}\')" class="unit-image minion-image" src="{1}"/>'.replace('{0}', minion.name.trim()).replace('{1}', 'assets/units/' + minion.id + '.svg');
            $(minion_list_template).appendTo('#minion-images')
            var minion_desc_template = "<div class='collapsable' id='{0}'>\
                                    <span class='minion-name'>{0}</span>\
                                    <p class='minion-desc'>\
                                    {1} <br/>\
                                    {2} <br/>\
                                    <span class='difficulty '>Difficulty: <span class={3}>{3}</span></span>\
                                    </p>\
                                </div>".replace(/\{0\}/g, minion.name.toUpperCase()).replace('{1}', factory.description).replace('{2}', factory.meta).replace(/\{3\}/g, factory.difficulty);
            $(minion_desc_template).appendTo('#minion-descs')
        }
        abilities = config_json.abilities;
        for (var index in abilities) {
            var ability = abilities[index];
            ability.name = ability.name.replace(' ', '-')
            var ability_list_template = '<img onclick="itemClicked(this, \'{0}\')" class="unit-image ability-image" src="{1}"/>'.replace('{0}', ability.name.trim()).replace('{1}', 'assets/units/' + ability.id + '.svg');
            $(ability_list_template).appendTo('#ability-images')
            var ability_desc_template = "<div class='collapsable' id='{0}'>\
                                    <span class='ability-name'>{0}</span>\
                                    <p class='ability-desc'>\
                                    {1} <br/>\
                                    {2} <br/>\
                                    <span class='difficulty '>Difficulty: <span class={3}>{3}</span></span>\
                                    </p>\
                                </div>".replace(/\{0\}/g, ability.name.toUpperCase()).replace('{1}', ability.description).replace('{2}', ability.meta).replace(/\{3\}/g, ability.difficulty);
            $(ability_desc_template).appendTo('#ability-descs')
        }
      }
    })
    
    
	var	$window = $(window),
		$body = $('body'),
		$wrapper = $('#wrapper'),
		$header = $('#header'),
		$footer = $('#footer'),
		$main = $('#main'),
		$main_articles = $main.children('article');

	// Breakpoints.
		breakpoints({
			xlarge:   [ '1281px',  '1680px' ],
			large:    [ '981px',   '1280px' ],
			medium:   [ '737px',   '980px'  ],
			small:    [ '481px',   '736px'  ],
			xsmall:   [ '361px',   '480px'  ],
			xxsmall:  [ null,      '360px'  ]
		});

	// Play initial animations on page load.
		$window.on('load', function() {
			window.setTimeout(function() {
				$body.removeClass('is-preload');
			}, 100);
		});

	// Fix: Flexbox min-height bug on IE.
		if (browser.name == 'ie') {

			var flexboxFixTimeoutId;

			$window.on('resize.flexbox-fix', function() {

				clearTimeout(flexboxFixTimeoutId);

				flexboxFixTimeoutId = setTimeout(function() {

					if ($wrapper.prop('scrollHeight') > $window.height())
						$wrapper.css('height', 'auto');
					else
						$wrapper.css('height', '100vh');

				}, 250);

			}).triggerHandler('resize.flexbox-fix');

		}

	   // Nav.
		var $nav = $header.children('nav'),
			$nav_li = $nav.find('li');

		// Add "middle" alignment classes if we're dealing with an even number of items.
			if ($nav_li.length % 2 == 0) {

				$nav.addClass('use-middle');
				$nav_li.eq( ($nav_li.length / 2) ).addClass('is-middle');

			}

	// Main.
		var	delay = 200,
        locked = false;

		// Show article
        $main._show = function(id, initial) {
				var $article = $main_articles.filter('#' + id);

				// No such article? Bail.
					if ($article.length == 0)
						return;

				// Handle lock.

					// Already locked? Speed through "show" steps w/o delays.
						if (locked || (typeof initial != 'undefined' && initial === true)) {

							// Mark as switching.
//								$body.addClass('is-switching');

							// Mark as visible.
								$body.addClass('is-article-visible');

							// Deactivate all articles (just in case one's already active).
								$main_articles.removeClass('active');

							// Hide header, footer.
								$header.hide();
								$footer.hide();

							// Show main, article.
								$main.show();
								$article.show();

							// Activate article.
								$article.addClass('active');

							// Unlock.
								locked = false;

							// Unmark as switching.
								setTimeout(function() {
									$body.removeClass('is-switching');
								}, (initial ? 1000 : 0));

							return;

						}

					// Lock.
						locked = true;

				// Article already visible? Just swap articles.
					if ($body.hasClass('is-article-visible')) {

						// Deactivate current article.
							var $currentArticle = $main_articles.filter('.active');

							$currentArticle.removeClass('active');

						// Show article.
							setTimeout(function() {

								// Hide current article.
									$currentArticle.hide();

								// Show article.
									$article.show();

								// Activate article.
									setTimeout(function() {

										$article.addClass('active');

										// Window stuff.
											$window
												.scrollTop(0)
												.triggerHandler('resize.flexbox-fix');

										// Unlock.
											setTimeout(function() {
												locked = false;
											}, delay);

									}, 25);

							}, delay);

					}

				// Otherwise, handle as normal.
					else {

						// Mark as visible.
							$body
								.addClass('is-article-visible');

						// Show article.
							setTimeout(function() {

								// Hide header, footer.
									$header.hide();
									$footer.hide();

								// Show main, article.
									$main.show();
									$article.show();

								// Activate article.
									setTimeout(function() {

										$article.addClass('active');

										// Window stuff.
											$window
												.scrollTop(0)
												.triggerHandler('resize.flexbox-fix');

										// Unlock.
											setTimeout(function() {
												locked = false;
											}, delay);

									}, 25);

							}, delay);

					}

			};
        // Hide article
        $main._hide = function(addState) {

				var $article = $main_articles.filter('.active');

				// Article not visible? Bail.
					if (!$body.hasClass('is-article-visible'))
						return;

				// Add state?
					if (typeof addState != 'undefined'
					&&	addState === true)
						history.pushState(null, null, '#');

				// Handle lock.

					// Already locked? Speed through "hide" steps w/o delays.
						if (locked) {

							// Mark as switching.
								$body.addClass('is-switching');

							// Deactivate article.
								$article.removeClass('active');

							// Hide article, main.
								$article.hide();
								$main.hide();

							// Show footer, header.
								$footer.show();
								$header.show();

							// Unmark as visible.
								$body.removeClass('is-article-visible');

							// Unlock.
								locked = false;

							// Unmark as switching.
								$body.removeClass('is-switching');

							// Window stuff.
								$window
									.scrollTop(0)
									.triggerHandler('resize.flexbox-fix');

							return;

						}

					// Lock.
						locked = true;

				// Deactivate article.
					$article.removeClass('active');

				// Hide article.
					setTimeout(function() {

						// Hide article, main.
							$article.hide();
							$main.hide();

						// Show footer, header.
							$footer.show();
							$header.show();

						// Unmark as visible.
							setTimeout(function() {

								$body.removeClass('is-article-visible');

								// Window stuff.
									$window
										.scrollTop(0)
										.triggerHandler('resize.flexbox-fix');

								// Unlock.
									setTimeout(function() {
										locked = false;
									}, delay);

							}, 25);

					}, delay);


			};
		// Articles close button
        $main_articles.each(function() {

            var $this = $(this);

            // Close.
                $('<div class="close">Close</div>')
                    .appendTo($this)
                    .on('click', function() {
                        location.hash = '';
                    });

            // Prevent clicks from inside article from bubbling.
                $this.on('click', function(event) {
                    event.stopPropagation();
                });

        });

		// Back to menu on outside click
        $body.on('click', function(event) {
                if ($body.hasClass('is-article-visible'))
                    $main._hide(true);

        });
    
        // Back to menu on ESC
        $window.on('keyup', function(event) {

            switch (event.keyCode) {
                case 27:
                    if ($body.hasClass('is-article-visible'))
                            $main._hide(true);

                    break;

                default:
                    break;

            }

        });

        $window.on('hashchange', function(event) {

				// Empty hash?
					if (location.hash == ''
					||	location.hash == '#') {

						// Prevent default.
							event.preventDefault();
							event.stopPropagation();

						// Hide.
							$main._hide();

					}

				// Otherwise, check for a matching article.
					else if ($main_articles.filter(location.hash).length > 0) {

						// Prevent default.
							event.preventDefault();
							event.stopPropagation();

						// Show article.
							$main._show(location.hash.substr(1));

					}

			});

		// Scroll restoration.
		// This prevents the page from scrolling back to the top on a hashchange.
			if ('scrollRestoration' in history)
				history.scrollRestoration = 'manual';
			else {

				var	oldScrollPos = 0,
					scrollPos = 0,
					$htmlbody = $('html,body');

				$window
					.on('scroll', function() {

						oldScrollPos = scrollPos;
						scrollPos = $htmlbody.scrollTop();

					})
					.on('hashchange', function() {
						$window.scrollTop(oldScrollPos);
					});

			}

		// Initialize.
        // Hide main, articles.
            $main.hide();
            $main_articles.hide();

        // Initial article.
            if (location.hash != ''
            &&	location.hash != '#')
                $window.on('load', function() {
                    $main._show(location.hash.substr(1), true);
                });
})