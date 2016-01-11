import EventEmitter from 'events';
import bindings from 'bindings';

const CANWrap = bindings('can_wrap').CANWrap;

class Socket extends EventEmitter {
  constructor(iface) {
    super();
    this._handle = new CANWrap();
    this._handle.onSent(() => { this._onSent(); });
    this._bound = false;
    this._sendQueue = [];
    if (typeof iface === 'string' || iface instanceof String) {
      this.bind(iface);
    }
  }

  bind(iface) {
    if (this._bound) {
      throw new Error('Socket is already bound');
    }

    const err = this._handle.bind(iface);
    if(err != 0) {
      throw new Error('Failed to bind: ' + err);
    }

    this._bound = true;
    return this;
  }

  send(id, buffer, callback) {
    if (typeof buffer === 'string') {
      buffer = new Buffer(buffer);
    } else if (!(buffer instanceof Buffer)) {
      throw TypeError('Second argument must be a Buffer or String');
    }

    // normalize callback so it is either a function or undefined
    if (typeof callback !== 'function') {
      callback = undefined;
    }

    const sending = this._sendQueue.length > 0;
    this._sendQueue.push({id: id, buffer: buffer, callback: callback});

    if (!sending) {
      this._handle.send(id, buffer);
    }
  }

  _onSent(err) {
    const sent = this._sendQueue[0];
    if (sent.callback) {
      sent.callback(err);
    }
    this._sendQueue.shift();
    const next = this._sendQueue[0];
    if (this._sendQueue.length > 0) {
      this._handle.send(next.id, next.buffer);
    }
  }
}

export default {
  Socket: Socket
}
