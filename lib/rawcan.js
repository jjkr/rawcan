import {EventEmitter} from 'events';
import bindings from 'bindings';

const CANWrap = bindings('can_wrap.node').CANWrap;

class Socket extends EventEmitter {
  constructor(iface) {
    super();
    this._handle = new CANWrap();
    this._handle.owner = this;
    this._handle.onSent((err) => { this._onSent(err); });
    this._handle.onMessage(
        (id, buffer) => { this.emit('message', id, buffer); });
    this._handle.onError((err) => { this.emit('error', err); });
    this._bound = false;
    this._sendQueue = [];
    if (typeof iface === 'string' || iface instanceof String) {
      this.bind(iface);
    }
  }

  get iface() { return this._iface; }

  bind(iface) {
    if (this._bound) {
      throw new Error('Socket is already bound');
    }

    this._healthCheck();

    const err = this._handle.bind(iface);
    if(err != 0) {
      throw new Error('Failed to bind: ' + err);
    }

    this._iface = iface;
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

    this._healthCheck();

    const sending = this._sendQueue.length > 0;
    this._sendQueue.push({id: id, buffer: buffer, callback: callback});

    if (!sending) {
      this._handle.send(id, buffer);
    }
  }

  setFilter(filter, mask) {
    this._healthCheck();
    this._handle.setFilter(filter, mask);
  }

  close() {
    this._healthCheck();
    this._handle.close();
    this._handle = undefined;
    //this.emit('close');
  }

  ref() {
    this._healthCheck();
    this._handle.ref();
  }

  unref() {
    this._healthCheck();
    this._handle.unref();
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

  _healthCheck() {
    if (!this._handle) {
      throw new Error('Not running');
    }
  }
}

export default {
  EFF_FLAG: 0x80000000, // extended frame format
  RTR_FLAG: 0x40000000, // remote transmission request
  ERR_FLAG: 0x20000000, // error

  SFF_MASK: 0x7FF, // standard frame format - 11 bits
  EFF_MASK: 0x1FFFFFFF, // extended frame format - 29 bits
  ERR_MASK: 0x1FFFFFFF,

  Socket: Socket,
  createSocket: (iface) => { return new Socket(iface); }
}
