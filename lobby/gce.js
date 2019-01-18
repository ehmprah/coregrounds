/**
 * @file
 * Manage and autoscale game server instances
 */
const Logger = require('./logger.js');
const ServerPool = require('./serverpool.js');
const Compute = require('@google-cloud/compute');
const configfile = require('./config.json');

const config = process.env.NODE_ENV === 'production'
  ? configfile.production
  : configfile.development;

const compute = new Compute(config.GCE.CLIENTCONFIG);
const vms = {};
let runManagementLoop = true;

if (process.env.NODE_ENV === 'production') {
  initializeInstances();
  setInterval(() => {
    // Update the metadata first
    updateMetadata();
    // Run the management loop 15 secs later
    setTimeout(manageInstances, 15000);
  }, 60000);
  // Restart the servers regularly
  setInterval(() => {
    Logger.log(`Performing scheduled restart...`);
    module.exports.restartServers();
  }, 3 * 86400 * 1000);
}

module.exports.setRegion = function setRegion(server) {
  if (process.env.NODE_ENV !== 'production') return;
  Object.keys(vms).forEach((id) => {
    const instance = vms[id];
    instance.getMetadata().then(() => {
      const ip = instance.metadata.networkInterfaces[0].accessConfigs[0].natIP;
      if (ip === server.ip) {
        server.region = id.match(/[a-z]*/)[0];
        Logger.log(`Server ${ip} in ${server.region.toUpperCase()} created`);
      }
    });
  });
};

module.exports.getStats = function getStats() {
  if (process.env.NODE_ENV !== 'production') return;
  let stats = 'Infrastructure Overview:';
  Object.keys(vms).forEach((id) => {
    const instance = vms[id];
    if (instance.metadata.status !== 'RUNNING') return;
    const ip = instance.metadata.networkInterfaces[0].accessConfigs[0].natIP;
    stats += `\nInstance "${id}" at ${ip}\nis ${instance.metadata.status}`;
    // List games
    const server = ServerPool.getServerAtIp(ip);
    if (server) {
      stats += `, has ${server.games.length} running games (${server.games.join(', ')})`;
      if (server.restart) {
        stats += `, is scheduled for restart`;
      }
    } else {
      stats += `, but server is not ready`;
    }
  });
  return stats;
};

module.exports.restartServers = function restartServers() {
  if (process.env.NODE_ENV !== 'production') return;
  // We turn off the regular loop until we've started the temp instances
  runManagementLoop = false;
  setTimeout(() => {
    runManagementLoop = true;
  }, 30000);
  // Boot temp instances and mark instances for restart
  Object.keys(config.GCE.ZONES).forEach((region) => {
    const failover = vms[`${region}-game-temp`];
    if (failover.metadata.status !== 'RUNNING') {
      failover.start().catch((error) => {
        Logger.log(`Error starting instance: ${JSON.stringify(error)}`);
      });
    }
    // Schedule restart in 30secs
    setTimeout(() => {
      // Set all servers to restart asap
      for (let i = 1; i <= config.GCE.MAX_INSTANCES; i++) {
        const instance = vms[`${region}-game-${i}`];
        if (instance.metadata.status === 'RUNNING') {
          const ip = instance.metadata.networkInterfaces[0].accessConfigs[0].natIP;
          const server = ServerPool.getServerAtIp(ip);
          if (server) server.scheduleRestart();
        }
      }
    }, 29000);
  });
  Logger.log(`Marking instances in all regions as ready for restart in 30 seconds.`);
};

function updateMetadata() {
  Object.keys(vms).forEach((id) => {
    vms[id].getMetadata();
  });
}

function manageInstances() {
  if (!runManagementLoop) return;
  const next = {};
  // Update the metadata
  Object.keys(vms).forEach((id) => {
    const instance = vms[id];

    // We don't act on instances that are currently starting or shutting down
    if (instance.metadata.status !== 'RUNNING' && instance.metadata.status !== 'TERMINATED') {
      return;
    }

    // Get the ip and the server object
    const ip = instance.metadata.networkInterfaces[0].accessConfigs[0].natIP;
    const server = ServerPool.getServerAtIp(ip);

    // If we don't have a server, we can assume an unresponsive instance
    if (!server) {
      if (instance.metadata.status === 'RUNNING') {
        Logger.log(`Restarting unresponsive instance: ${id}`);
        instance.reset().catch((error) => {
          Logger.log(`Error restarting instance: ${JSON.stringify(error)}`);
        });
      }
      return;
    }

    // Set low priority for temp instances
    if (id.match(/(temp)/)) {
      server.priority = 0;
    }

    // Process scheduled restarts
    if (server.restart) {
      if (server.load() === 0) {
        Logger.log(`Scheduled restart instance: ${id}`);
        instance.reset().catch((error) => {
          Logger.log(`Error restarting instance: ${JSON.stringify(error)}`);
        });
      } else {
        return;
      }
    }

    // Health check
    if (config.GCE.AUTORESTART) {
      if (instance.metadata.status === 'RUNNING') {
        if (server.ready) {
          server.healthy = true;
        } else {
          if (server.healthy) {
            server.healthy = false;
          } else {
            Logger.log(`Restarting unresponsive instance: ${id}`);
            instance.reset().catch((error) => {
              Logger.log(`Error restarting instance: ${JSON.stringify(error)}`);
            });
          }
          return;
        }
      }
    }

    // Load balancing
    const load = server.load();
    if (next[server.region] === 'stop' && load === 0) {
      Logger.log(`Stopping unneeded instance: ${id}`);
      instance.stop().catch((error) => {
        Logger.log(`Error stopping instance: ${JSON.stringify(error)}`);
      });
    } else if (next[server.region] === 'start' && instance.metadata.status === 'TERMINATED') {
      Logger.log(`Started new instance: ${id}`);
      instance.start().catch((error) => {
        Logger.log(`Error starting instance: ${JSON.stringify(error)}`);
      });
      next[server.region] = '';
    } else if (load >= 0.75 || server.restart) {
      next[server.region] = 'start';
    } else if (load < 0.75) {
      next[server.region] = 'stop';
    }
  });
}

function initializeInstances() {
  Object.keys(config.GCE.ZONES).forEach((region) => {
    const zone = compute.zone(config.GCE.ZONES[region]);
    // At the beginning, we get all machines, creating them if necessary
    for (let i = 1; i <= config.GCE.MAX_INSTANCES; i++) {
      createInstance(zone, `${region}-game-${i}`);
    }
    // Initialize temporary instance for scheduled restarts
    createInstance(zone, `${region}-game-temp`);
  });
}

function createInstance(zone, id) {
  vms[id] = zone.vm(id);
  vms[id].get({
    autoCreate: true,
    machineType: config.GCE.INSTANCE_TYPE,
    disks: [{
      boot: true,
      autoDelete: true,
      initializeParams: {
        sourceImage: config.GCE.INSTANCE_IMAGE,
      },
    }],
    networkInterfaces: [{
      accessConfigs: [{
        type: 'ONE_TO_ONE_NAT',
        name: 'External NAT',
      }],
      network: 'global/networks/default',
    }],
    tags: ['game-server'],
  }).catch((error) => {
    Logger.log(`Error creating instance: ${JSON.stringify(error)}`);
  });
}
