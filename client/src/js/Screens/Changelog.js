/**
 * @file
 * The rolling changelog
 */
import Icons from './Elements/Icons';

Screens.add('changelog-1.0.6', () => {
  Screens.update('changelog-1.0.6', `
    <div class="container-full">
      <button class="show-menu" onclick="showMenu()">${Icons.get('menu')}</button>
      <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
      <div class="content-text">
        <h2>Patch Notes 1.0.6: MOAR Experience!</h2>

        <ul class="changes">
          <li class="added">
            We halved the prices for all cordium packages. To give back to early supporters, we
            have retroactively TRIPLED the rewards all purchases made so far. So if you haven't
            purchased Cordium so far, you can now do so at half the price. If you did, you now have
            three times the value for your money.
          </li>
          <li class="added">
            Added the "Training Grounds" to the home screen, where we'll list tutorial and strategy
            videos by us and the community. If you've made an (english) tutorial, send it our way!
          </li>
          <li class="added">
            Added profanity filter for nicknames
          </li>
          <li class="added">
            Added new side menu and cleaned up the home screen
          </li>
          <li class="changed">
            The LobbyBot on our Discord no longer posts in the #matchmaking channel, but the newly
            created #queues channel. The #matchmaking channel should be used only by humans looking
            for a match.
          </li>
          <li class="changed">
            Matchmaking is now locked until players are level 3
          </li>
          <li class="changed">
            Added additional screen to the tutorial explaining the XP system
          </li>
          <li class="changed">
            Cores are now rendered above units, making core health more visible
          </li>
        </ul>

        <h3>Balancing</h3>
        <ul class="changes">
          <li class="buffed">Increased XP per second from 0.4 to 1</li>
          <li class="buffed">Increased XP bonus for units from 2% to 3% per level</li>
          <li class="buffed">CORE now does 20 damage per second to adjacent enemies.</li>
          <li class="buffed">DEFLECTOR increased spawn speed from 0.071 to 0.085</li>
          <li class="buffed">TANK increased base health from 350 to 400</li>
          <li class="buffed">PHASER increased damage from 100 to 110</li>
          <li class="nerfed">PHASER reduced incoming XP from 80% to 50%</li>
          <li class="nerfed">CHAKRAM reduced damage from 120 to 110</li>
          <li class="nerfed">CHAKRAM reduced attack speed from 0.166 to 0.15</li>
          <li class="nerfed">Tower death cooldown increased from 7 to 10 seconds</li>
        </ul>

        <h3>Bugfixes</h3>
        <ul class="changes">
          <li class="fixed">Added missing click sound for news and video on the home screen</li>
          <li class="fixed">Fixed missing "Heart of Gold" icon for the indie supporter package</li>
          <li class="fixed">Added missing close button when starting Cordium transaction</li>
          <li class="fixed">The continue button at the end of the game no longer competes with towers below</li>
        </ul>

        <button class="box previous" onclick="Screens.show('changelog-1.0.5')">PREVIOUS PATCH</button>
      </div>
    </div>
  `);
});

Screens.add('changelog-1.0.5', () => {
  Screens.update('changelog-1.0.5', `
    <div class="container-full">
      <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
      <div class="content-text">
        <h2>Patch Notes 1.0.5: Growing roots!</h2>

        <p>
          UPDATE 18/02/07: We've also fixed a bunch of crash reasons and startup issues across all
          platforms.
        </p>

        <p>
          Our community on Discord has been very active and keeps coming up with more ideas than
          we have time to implement! Tauntos for example had the idea for a ROOT ability, which we
          implemented with this patch (we have unlocked this one for you, Tauntos). We also want to
          thank BOB, Shurutsue, Titan, Lotus and everyone else for your suggestions. This
          patch is all yours!
        </p>

        <h3>New Ability</h3>
        <ul class="changes">
          <li class="added">
            ROOT ability: Enemies hit cannot move for 5s. 75s cooldown.
          </li>
        </ul>

        <h3>Changes</h3>
        <ul class="changes">
          <li class="added">
            NOOB PROTECTION Until you're level 3, you'll only be matchmade with other newbies
            below level 3.
          </li>
          <li class="added">
            During the draft and in the game guide you now see against which other units a unit
            is particularly weak or strong
          </li>
          <li class="changed">
            Reduced the minimum time you have to spend in the queue before the cross-regional
            matchmaking kicks in from 360 to 30 seconds
          </li>
          <li class="changed">
            Cross-regional is now turned on by default for new players
          </li>
          <li class="changed">
            Private matches no longer count towards the casual leaderboard
          </li>
          <li class="changed">
            PVP matches now yield 20% more credits and experience than bot matches
          </li>
          <li class="fixed">
            Fixed wrong duration in lifelink description
          </li>
        </ul>

        <h3>Balancing</h3>
        <ul class="changes">
          <li class="buffed">Increased SNIPER range from 1.6 to 1.7 tiles</li>
          <li class="nerfed">Decreased SUNBEAM range from 1.8 to 1.6 tiles</li>
          <li class="nerfed">Decreased SUNBEAM damage from 96 to 64</li>
          <li class="nerfed">Increased IONSTRIKE cooldown from 75s to 90s</li>
          <li class="nerfed">CHAKRAM gets no xp bonus on attack speed and damage anymore</li>
        </ul>

        <button class="box previous" onclick="Screens.show('changelog-1.0.4')">PREVIOUS PATCH</button>
      </div>
    </div>
  `);
});

Screens.add('changelog-1.0.4', () => {
  Screens.update('changelog-1.0.4', `
    <div class="container-full">
      <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
      <div class="content-text">
        <h2>Patch Notes 1.0.4: Onboarding Galore!</h2>

        <h3>New features and things</h3>
        <ul class="changes">
          <li class="added">
            The SUNBEAM has returned to the Coregrounds!
          </li>
          <li class="added">
            Added link to game guide on unlock screens
          </li>
          <li class="added">
            Added invitation to join <a href="https://discord.gg/FsjDWu9" style="font-weight:bold;">
            #matchmaking</a> channel in the matchmaking menu
          </li>
          <li class="added">
            The <a href="https://discord.gg/FsjDWu9" style="font-weight:bold;">#matchmaking</a>
            channel will be notified when players join the queue
          </li>
          <li class="added">
            We've unlocked the "Good Game" and "Good Luck, Have Fun" emotes for everyone, just
            because we're nice and the Coregrounds should be as well!
          </li>
          <li class="added">
            We've added the ALT/CTRL/SHIFT + 1-6 shortcut. You can now press ALT + 1-6 to upgrade
            and CTRL/SHIFT + 1-6 to spawn.
          </li>
          <li class="added">
            Last patch, we added a new onboarding process which allows all new players to choose 3
            free modifications to get started. We added more free stuff in this patch: all new
            players get a free tower, minion and ability at the start. All existing players have
            been granted 4500 credits in compensation.
          </li>
          <li class="added">
            Improved onboarding flow: we added another info screen and reduced the steps to start
            the first match.
          </li>
          <li class="added">
            Added tip bubbles at the start of a match prompting you to build walls, build towers
            and spawn minions.
          </li>
        </ul>

        <h3>Less buggy things</h3>
        <ul class="changes">
          <li class="fixed">
            Fixed bug where you could earn the rush achievement when losing
          </li>
          <li class="fixed">
            The farsight icon now has opacity on the map for better visibility
          </li>
          <li class="fixed">
            Fixed bug where the UI could get locked at the end of a game
          </li>
          <li class="fixed">
            Fixed ingame emote positions for the right side player
          </li>
        </ul>

        <h3>Balanced things</h3>
        <ul class="changes">
          <li class="buffed">Decreased DYNABLASTER cooldown from 300 to 180 seconds</li>
        </ul>
        <ul class="changes">
          <li class="nerfed">Decreased LIFELINK effect duration from 10 to 7 seconds</li>
        </ul>
        <ul class="changes">
          <li class="nerfed">Reworked REPEATER damage from 40/60/75/85/90 to 35/40/50/65/85</li>
        </ul>

        <button class="box previous" onclick="Screens.show('changelog-1.0.2')">PREVIOUS PATCH</button>
      </div>
    </div>
  `);
});

Screens.add('changelog-1.0.3', () => {
  Screens.update('changelog-1.0.3', `
    <div class="container-full">
      <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
      <div class="content-text">
        <h2>Patch Notes 1.0.3: All aboard</h2>
        <img src="img/other/dynablaster.gif">

        <h3>Such new stuffs! Many content! Wow!</h3>
        <ul class="changes">
          <li class="added">Keyboard Shortcut: CTRL + Click on a minion factory will spawn it</li>
          <li class="added">Keyboard Shortcut: ALT + Click on unit factory upgrades the lowest stat</li>
          <li class="added">Modification: +2% Attack Speed (Global)</li>
          <li class="added">Modification: +2% Damage (Global)</li>
          <li class="added">Modification: +2% Health (Global)</li>
          <li class="added">Modification: +2% Thorns (Global)</li>
          <li class="added">Modification: +5% Vision Range (Global)</li>
          <li class="added">Ability: DYNABLASTER. Destroy any wall and all edge adjacent walls after 1s. 300s cooldown (see GIF at the top).</li>
        </ul>

        <h3>It's the little things</h3>
        <ul class="changes">
          <li class="changed">
            Reworked the onboarding experience for new users; all new users get 3 free
            modifications during the new onboarding process to fill their first mod page before
            their first game. All existing players have been granted 3000 credits, yay!
          </li>
          <li class="changed">
            When a match starts, a notification sound is played, the window is focused and the
            window title is changed. If you have to wait in queue, you can do
            different things now without missing the start of the match!
          </li>
          <li class="changed">Updated global target icon</li>
          <li class="changed">During the ban phase, all units are shown in grey</li>
          <li class="changed">During the pick phase, the daily free picks are highlighted</li>
          <li class="changed">Added minion drop animation when using the spacebar to spawn</li>
        </ul>

        <h3>Giant bugs squatted</h3>
        <ul class="changes">
          <li class="fixed">
            Fixed bug where microtransactions could not be completed
          </li>
          <li class="fixed">
            Modification bonuses are now properly applied ingame
          </li>
          <li class="fixed">
            Streamlined the account name validation between webservice and game client. It is no
            longer possible to create usernames with invalid characters, which would lead to error
            messages down the road.
          </li>
          <li class="fixed">
            Fixed bug where closing the achievement notification closed the match history as well
          </li>
          <li class="fixed">
            Pressing spacebar without any minions to spawn no longer makes the spawn sound
          </li>
          <li class="fixed">
            Fixed bug where the time left when selecting a modification page was only shown for
            players with more than one page
          </li>
          <li class="fixed">
            Pressing ENTER when editing a modification page now properly saves it
          </li>
          <li class="fixed">
            When joining a private match, the input field is automatically focused and you can
            submit with ENTER
          </li>
          <li class="fixed">
            HEALER and HEALRAY no longer target themselves
          </li>
        </ul>

        <h3>Balancing</h3>
        <ul class="changes">
          <li class="buffed">Increased free walls at the start of a match from 7 to 9</li>
          <li class="buffed">Increased XP for tower kills from 50 to 75</li>
          <li class="buffed">Increased XP per second from 0.3 to 0.4</li>
          <li class="nerfed">ADRENALINE no longer benefits from the 1.5 healing modifier on cores</li>
        </ul>

        <button class="box previous" onclick="Screens.show('changelog-1.0.2')">PREVIOUS PATCH</button>
      </div>
    </div>
  `);
});

Screens.add('changelog-1.0.2', () => {
  Screens.update('changelog-1.0.2', `
    <div class="container-full">
      <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
      <div class="content-text">
        <h2>Patch Notes 1.0.2</h2>

        <h3>Changes</h3>
        <ul class="changes">
          <li class="added">You can now toggle fullscreen in the settings and with the F11 key</li>
          <li class="added">Added cooldown info and improved descriptions for abilities</li>
          <li class="added">When at the minion limit, all your minion factories are now highlighted in red</li>
          <li class="changed">Made the minion limit "middle" more visible in the interface bar</li>
          <li class="changed">You can now view player profiles when clicking their name in the match history</li>
        </ul>

        <h3>Bugfixes</h3>
        <ul class="changes">
          <li class="fixed">The player level up notification is now properly displayed</li>
          <li class="fixed">The client is no longer unresponsive when deselecting an ability with ESC</li>
          <li class="fixed">You can no longer re-enter the play menu when you're in the matchmaking queue</li>
          <li class="fixed">After a connection loss, the menu close button appears properly when reconnecting</li>
          <li class="fixed">Added missing player icon "Beta Phaser"</li>
        </ul>

        <h3>Balancing</h3>
        <ul class="changes">
          <li class="fixed"><span class="name">SHURIKEN</span>Projectile bounce range reduced from 1 to 0.6 tiles</li>
          <li class="fixed"><span class="name">CHAKRAM</span>Projectile bounce range reduced from 1 to 0.6 tiles</li>
          <li class="fixed"><span class="name">CHAKRAM</span>XP per damage dealt reduced from 0.4% to 0.2%</li>
        </ul>

        <button class="box previous" onclick="Screens.show('changelog-1.0.1')">PREVIOUS PATCH</button>
      </div>
    </div>
  `);
});

Screens.add('changelog-1.0.1', () => {
  Screens.update('changelog-1.0.1', `
    <div class="container-full">
      <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
      <div class="content-text">
        <h2>Patch Notes 1.0.1</h2>
        <h3>Attack the lag: two new regions!</h3>
        <p>We added two new server regions and settings where you can choose your gameplay region.
        <span class="notes">We've also added a option to allow matchmaking across regions; during
        Early Access, we might not have the player numbers to fill three regions. With the
        cross-regional matchmaking opt-in, we get the best of both worlds: better overall latency
        with an option to find additional opponents in other regions. Even with this setting enabled
        the game will still try to find an opponent from your region, and will only start matching
        you across regions after five minutes in the queue.</span></p>

        <h3>OP, nerf pls: Some balancing</h3>
        <p>With your help, we've identified a few overperformers and applied a few nerfs:</p>
        <ul class="changes">
          <li class="nerfed"><span class="name">CORE</span>Regeneration reduced from 1% to 0.5% of total health per second</li>
          <li class="nerfed"><span class="name">CHAKRAM</span>Damage reduced from 150 to 120</li>
          <li class="nerfed"><span class="name">HEAL</span>Cooldown increased from 20 to 30 seconds</li>
          <li class="nerfed"><span class="name">RADIATOR</span>Radiator health reduced from 1000 to 750</li>
        </ul>

        <h3>n00b protection: early bot levels less hard</h3>
        <p>We've reduced the difficulty for the first bot levels.</p>

        <button class="box previous" onclick="Screens.show('changelog-1.0.0')">PREVIOUS PATCH</button>
      </div>
    </div>
  `);
});

Screens.add('changelog-1.0.0', () => {
  Screens.update('changelog-1.0.0', `
    <div class="container-full">
      <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
      <div class="content-text">
        <h2>Patch Notes 1.0.0</h2>
        <p>Almost two years in the making, the release version is finally here! We've come a long
        way from a browser-based alpha to a full-fledged game on Steam and the architecture behind
        the game has been completely revamped to and is only waiting to be scaled. We're incredibly
        glad that you're here and hope that you have a fun time on the Coregrounds!</p>

        <h3>Early Access Caveats</h3>
        <p>You're the first ones on the new Coregrounds. You will experience bugs and server crashes
        or other weird issues. Please help us fix those problems by reporting when and how exactly
        you're encountering them.</p>
        <p>One word on lag to start: as long as we don't have enough players to populate multiple
        regions, we will have to work with one server region, especially now in the beta. This
        means for some people there's going to be lag, but there's really no way around it for now,
        it's a big world for a little indie game!</p>

        <h3>Hello Darkness my old friend</h3>
        <p>Reworked vision system: all units can see less far, but your vision is lined with
        half-vision tiles where you see enemy walls, but no towers and minions.<span class="notes">
        So far, vision was too easily gained with walls. Now you'll have to use your units to gain
        and control vision, which will make the game strategically much more interesting and
        diverse!</span></p>

        <h3>Play to win, pay for looks</h3>
        <p>We have renamed the old currency to credits and have introduced a new premium currency to
        the game. Everything gameplay related you can still unlock just by playing the game.
        <span class="notes">You will be able to buy Cordium with in-app purchases and use it to buy
        credits, boosts for credits as well as all kinds of vanity items like colors or skins. You
        don't pay to win, you pay to visually stand out on the Coregrounds and to customize the
        game to your liking!</p>

        <h3>Veteran Added Levels</h3>
        <p>We've added player experience levels. Currently there are no rewards for these yet, but
        we will maybe change that in the future. <span class="notes">We added the player level to
        have a simple stat which persists across seasons. Your rank might change over seasons, but
        you will be able to showcase your veteran status with the experience level!</span></p>

        <h3>Botilicious: The 100</h3>
        <p>We have rebuilt the bot you can hone your skills against. Its bot difficulty now scales
        from level 1 to 100, with new achievements to earn for beating them as well.
        <span class="notes">We're looking forward to your feedback on the bot difficulty on all
        levels! Let us know what you think!</span></p>

        <h3>Shield Generator becomes the Guardian</h3>
        <p>The Shield Generator has been reworked and now is the Guardian, which grants additional
        armor to nearby allies. <span class="notes">The Guardian still serves the same purpose as
        the Shield Generator, but is clearly different from the Shield ability now.</span></p>

        <h3>New Ability: Exhaust</h3>
        <p>We've added a new ability, Exhaust, which reduces the attack speed of all enemies hit.
        <span class="notes">We've completed the holy quadruplicity of Slow, Hurry, Frenzy and
        Exhaust!</span></p>

        <h3>Benched, for now</h3>
        <p>Teleport, Transceiver, Shotgun and Sunbeam have been disabled for technical reasons for
        now. We will revisit these units in the future.</p>

        <h3>Good colors and bad colors</h3>
        <p>Several colors have been removed from the game or from the regular unlocks. You've been
        reimbursed if you had these unlocked. The good news: we've added a bunch of new ones!</p>

        <button class="box previous" onclick="Screens.show('changelog-0.9.21')">PREVIOUS PATCH</button>
      </div>
    </div>
  `);
});

Screens.add('changelog-0.9.21', () => {
  Screens.update('changelog-0.9.21', `
    <div class="container-full">
      <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
      <div class="content-text">
        <h2>Patch Notes 0.9.21</h2>
        <h3>Not sticking around: Gameplay changes</h3>
        <ul class="changes">
        <li class="changed">Building or moving a tower on a wall affected by Demolish or Wallswap will clear the effect and keep the wall alive.</li>
        </ul>

        <h3>Oh that regen: Balancing</h3>
        <ul class="changes">
        <li class="nerfed">Health regeneration modification value reduced from 0.2% to 0.04%. <span class="notes">This may seem like a very drastic nerf, but what's actually drastic is its former value: whereas all other modifications gave you a bonus of around 20% to a given stat, you could effectively more than double the health regeneration for many units with this modification. With this change, it's more in line with the other modifications.</span></li>
        <li class="nerfed"><span class="name">Hammerling</span>Armor reduced from 25 to 15</li>
        <li class="nerfed"><span class="name">Shield Generator</span>Armor reduced from 25 to 15</li>
        </ul>

        <h3>More stacks, less stacks: Bug fixes</h3>
        <ul class="changes">
        <li class="fixed">The armor penetration modification does work now.</li>
        <li class="fixed"><span class="name">Container</span> no longer deals damage while still on the core. <span class="notes">It was possible to stack those for insane damage, making it almost impossible for the other player to end the game without ranged units.</span></li>
        </ul>
      </div>
    </div>
  `);
});
