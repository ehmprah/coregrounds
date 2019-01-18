function LobbySocket(url) {
  // We keep our callbacks close
  this.callbacks = {};
  // Create WebSocket connection
  this.socket = new WebSocket(url);
  // Handle incoming messages
  this.socket.onmessage = payload => this.event(payload);
  // We emulate
  this.socket.onerror = () => this.event({ data: '{"event":"error"}' });
  this.socket.onclose = () => this.event({ data: '{"event":"close"}' });
  this.socket.onopen = () => this.event({ data: '{"event":"open"}' });
}

LobbySocket.prototype.on = function on(event, callback) {
  this.callbacks[event] = this.callbacks[event] || [];
  this.callbacks[event].push(callback);
  return this;
};

LobbySocket.prototype.event = function event(payload) {
  const msg = JSON.parse(payload.data);
  const chain = this.callbacks[msg.event];
  if (chain === undefined) return;
  chain.forEach((callback) => {
    callback(msg.data);
  });
  return this;
};

LobbySocket.prototype.close = function close() {
  this.socket.close();
};

LobbySocket.prototype.emit = function emit(event, data) {
  this.socket.send(JSON.stringify({
    event,
    data,
  }));
  return this;
};

export default {
  connect(url) {
    return new LobbySocket(url);
  },
};
