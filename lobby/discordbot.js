/**
 * @file
 * Control the lobby server from the discord chat
 */
const Discord = require('discord.js');
const GCE = require('./gce.js');
const ServerPool = require('./serverpool.js');
const Logger = require('./logger.js');
const config = require('./config.json');

const client = new Discord.Client();

const msgs = {
  // #infrastructure
  '425282616003002373': [],
  // #matchmaking
  '470898279811645460': [],
};

module.exports.send = function send(msg, channelId = '425282616003002373') {
  msgs[channelId].push(msg);
};

setInterval(() => {
  Object.keys(msgs).forEach((channelId) => {
    const channel = client.channels.get(channelId);
    if (channel && msgs[channelId].length) {
      channel.send(msgs[channelId].shift()).catch(e => Logger.log(`Discord error: ${e}`));
    }
  });
}, 500);

if (process.env.NODE_ENV === 'production') {
  client.on('ready', () => {
    // This event will run if the bot starts, and logs in, successfully.
    client.user.setActivity(`Management Bot`);
  });

  client.on('message', async (message) => {
    // Ignore messages from other bots
    if (message.author.bot) return;
    // Ignore messages without our prefix
    if (message.content.indexOf('!') !== 0) return;
    // Only listen in the infrastructure channel
    if (message.channel.name !== 'infrastructure') return;

    // Display infrastructure stats
    if (message.content === '!stats') {
      const stats = GCE.getStats();
      message.channel.send(stats).catch(e => Logger.log(`Discord error: ${e}`));
    }

    // Restart game servers
    if (message.content === '!restart') {
      GCE.restartServers();
    }

    // Update config
    if (message.content === '!config') {
      ServerPool.updateConfig();
      message.channel.send('Updating config for all game servers.')
        .catch(e => Logger.log(`Discord error: ${e}`));
    }
  });

  client.on('error', (e) => {
    Logger.log(`Discord error: ${e}`);
  });

  client.login(config.production.DISCORD_TOKEN);
}
