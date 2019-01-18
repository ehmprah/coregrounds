/**
 * @file
 * Template for the play menu
 *
 * TODO include queue here so we don't have to disconnect and reconnect on back button
 * TODO also, include a back button :-)
 */
import Event from '../Event';
import LobbyServer from '../Network/LobbyServer';
import Analytics from '../Analytics';
import Messages from './Elements/Messages';
import Queue from '../Screens/Elements/Queue';
import Sound from '../Sound';
import Session from '../Session/Session';
import { submitHandler } from '../Util/DOM';
import Icons from '../Screens/Elements/Icons';

Screens.add('play', () => {
  if (LobbyServer.isConnected()) {
    // If this is the first time a player clicks on the play button, we display a different page
    const account = Session.getAccount();
    if (account.season.bot.wins === 0 && account.previous.bot.wins === 0) {
      Screens.update('play', `
        <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
        <div class="container-flex">
          <div class="blackbox overlaybox">
            <p>
              One last tip before you play your first match: bot matches work the same way as
              multiplayer matches, so you don't have unlimited time to look at all the units in the
              pick and ban phase.
            </p>
            <p>
              But the first bot levels are really easy, you don't have to know all the units.
              Just try any unit, you'll get to know them as you play!
            </p>
            <div class="flex">
              <button class="box wide" id="btn-ranked-bot">
                ALRIGHT, LET'S PLAY
              </button>
            </div>
          </div>
        </div>
      `);

      $('#btn-ranked-bot').on('click', () => {
        // Start a bot game
        LobbyServer.emit('ClientStartBotGame');
        // Track queue start
        Analytics.event('Game', 'start', 'bot');
        // Display throbber message
        Screens.update('botgame', `
          <throbber>CREATING GAME</throbber>
        `);
      });
    } else {
      const matchmaking = Session.getAccount().level.current < 3
        ? `<button disabled class="box" onclick="Screens.show('matchmaking')">Matchmaking</button>
           <div style="color:#888;text-align:center; font-size: 14px; margin: -10px 0 10px 0;">
            Matchmaking is unlocked at level 3
           </div>`
        : `<button class="box" onclick="Screens.show('matchmaking')">Matchmaking</button>`;
      Screens.update('play', `
        <button class="show-menu" onclick="showMenu()">${Icons.get('menu')}</button>
        <button class="close" onclick="Screens.close()">${Icons.get('close')}</button>
        <div class="menu">
          ${matchmaking}
          <button class="box" onclick="Screens.show('botgame')">Play against Bot</button>
          <button class="box" onclick="Screens.show('private')">Private</a>
        </div>
      `);
    }
  } else {
    // Check if we have a region
    const account = Session.getAccount();
    if (!account.region.length) {
      Messages.show('Please choose gameplay region');
      return Screens.show('settings-region');
    }
    LobbyServer.connect();
    Screens.update('play', `
      <throbber>CONNECTING TO SERVER</throbber>
    `);
  }
});

Event.on('network ready', () => {
  // Display the play menu
  Screens.refresh();
});

Event.on('showing home', () => {
  // Disconnect as soon as we leave the play menu (which can also happen via
  // errors, which is why we catch it upon going home).
  if (LobbyServer.isConnected() && !Queue.active()) {
    LobbyServer.disconnect();
  }
});

Screens.add('botgame', () => {
  Screens.update('botgame', `
    <div class="menu">
      <button class="box" id="btn-ranked-bot">Ranked Bot Match</button>
      <button class="box" onclick="Screens.show('setup-botgame')">Custom Bot Match</button>
      <button class="box grey" onclick="Screens.show('play')">Back</button>
    </div>
  `);

  $('#btn-ranked-bot').on('click', () => {
    // Start a bot game
    LobbyServer.emit('ClientStartBotGame');
    // Track queue start
    Analytics.event('Game', 'start', 'bot');
    // Display throbber message
    Screens.update('botgame', `
      <throbber>CREATING GAME</throbber>
    `);
  });
});

Screens.add('setup-botgame', () => {
  Screens.update('setup-botgame', `
    <div class="menu">
      <input type="number" min="1" max="100" id="botlevel" placeholder="Enter bot level 1-100" style="margin-bottom: 1em; text-align:center;">
      <button class="box" id="btn-start-match">Start match</a>
      <button class="box grey" onclick="Screens.show('botgame')">Back</button>
    </div>
  `);

  $('#btn-start-match').on('click', () => {
    LobbyServer.emit('ClientStartCustomBotGame', { botlevel: $('#botlevel').value });
    // Track queue start
    Analytics.event('Game', 'start', 'custombot', $('#botlevel').value);
    // Display throbber message
    Screens.update('setup-botgame', `
      <throbber>CREATING GAME</throbber>
    `);
  });
});

Screens.add('matchmaking', () => {
  Screens.update('matchmaking', `
    <div class="menu">
      <div class="blackbox" style="color: #888;font-size: 16px;margin: 0 0 1em;padding: 1em;">
        The <a href="https://discord.gg/FsjDWu9" style="font-weight:bold;">#matchmaking</a>
        channel on our Discord server also is a great way to find people to play with!
      </div>
      <button class="box" id="btn-casual">Casual</button>
      <button disabled class="box" id="btn-ranked">Ranked</button>
      <div style="color:#888;text-align:center; font-size: 14px; margin: -10px 0 10px 0;">
        Ranked matches disabled during Early Access
      </div>
      <button class="box grey" onclick="Screens.show('play')">Back</button>
    </div>
  `);

  $('#btn-casual').on('click', () => {
    LobbyServer.emit('ClientJoinQueue', 'casual');
    Analytics.event('Game', 'queue', 'casual');
    Screens.update('matchmaking', `
      <throbber>JOINING QUEUE</throbber>
    `);
  });

  $('#btn-ranked').on('click', () => {
    LobbyServer.emit('ClientJoinQueue', 'ranked');
    Analytics.event('Game', 'queue', 'ranked');
    Screens.update('matchmaking', `
      <throbber>JOINING QUEUE</throbber>
    `);
  });
});

Screens.add('private', () => {
  Screens.update('private', `
    <div class="menu">
      <button class="box" onclick="Screens.show('private-join')"><span>Join</span></a>
      <button class="box" id="btn-casual"><span>Create</span></a>
      <button class="box grey" onclick="Screens.show('play')">Back</button>
    </div>
  `);

  $('#btn-casual').on('click', () => {
    LobbyServer.emit('ClientPrivateCreate', 'private');
    // Track queue start
    Analytics.event('Game', 'create', 'private');
    // Display throbber message
    Screens.update('private', `
      <throbber>CREATING GAME</throbber>
    `);
  });

  // $('#btn-random').on('click', () => {
  //   LobbyServer.emit('ClientPrivateCreate', 'random');
  //   // Track queue start
  //   Analytics.event('Game', 'create', 'random');
  //   // Display throbber message
  //   Screens.update('private', `
  //     <throbber>CREATING GAME</throbber>
  //   `);
  // });
});

Event.on('network ready', () => {
  // Add handling for successfully created matches
  LobbyServer.on('ClientPrivateMatchReady', (mid) => {
    Screens.update('private', `
      <div class="menu">
        <throbber>MATCH READY</throbber>
        <h2>Match ID: ${mid}</h2>
        <p>Waiting for opponent...</p>
        <button class="box grey" id="btn-cancel">Cancel</button>
      </div>
    `);
    $('#btn-cancel').on('click', () => {
      LobbyServer.emit('ClientPrivateCancel');
      // Track queue start
      Analytics.event('Game', 'cancel', 'private');
      // Go back to play menu
      Screens.show('play');
    });
  });

  // Add error message for invalid match ids.
  LobbyServer.on('ClientPrivateMatchNotFound', () => {
    Messages.show('Match not found', 'error');
    Sound.play('error');
  });

  // Add error message for invalid match ids.
  LobbyServer.on('ClientPrivateMatchIdInvalid', () => {
    Messages.show('Invalid match ID.', 'error');
    Sound.play('error');
  });

  // Handle queue start
  LobbyServer.on('ClientQueueConfirm', (estimate) => {
    Queue.start(estimate);
    Screens.show('home');
  });
});

Screens.add('private-join', () => {
  Screens.update('private-join', `
    <div class="menu">
      <form id="private-join">
        <input type="text" id="mid" value="" placeholder="Enter match ID" style="margin-bottom: 1em;">
        <button class="box" type="submit" id="btn-join">Join</a>
        <button class="box grey" onclick="Screens.show('private')">Back</button>
        <input type="submit" style="visibility: hidden; position: absolute;" />
      </form>
    </div>
  `);

  $('#mid').focus();
  submitHandler('btn-join', false, () => {
    $('#btn-join').classList.remove('waiting');
    LobbyServer.emit('ClientPrivateJoin', { mid: $('#mid').value });
    // Track queue start
    Analytics.event('Game', 'join', 'private');
    // Display throbber message
    Screens.update('private', `
      <throbber>SEARCHING GAME</throbber>
    `);
  });
});
