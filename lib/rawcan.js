import EventEmitter from 'events';
import bindings from 'bindings';

const CANWrap = bindings('can_wrap').CANWrap;

class Socket extends EventEmitter {
  constructor() {
    super();
    this._handle = new CANWrap();
    this._receiving = false;
    this._sendQueue = [];
  }

  bind(iface) {
    if (this._receiving) {
      throw new Error('Socket is already bound');
    }

    const err = this._handle.bind(iface);
    if(err != 0) {
      throw new Error('failed to bind: ' + err);
    }

    //this._handle.recvStart();
    this._receiving = true;

    return this;
  }

  send(id, buffer, callback) {
    const sending = this._sendQueue.length > 0;
    this._sendQueue.push([id, buffer, callback]);

    if (typeof buffer === 'string') {
      buffer = new Buffer(buffer);
    } else if (!(buffer instanceof Buffer)) {
      throw TypeError('Second argument must be a Buffer or String');
    }

    // normalize callback so it is either a function or undefined
    if (typeof callback !== 'function') {
      callback = undefined;
    }

    if (!sending) {
    }
  }

  _onSent(err) {
  }
}

export default {
  Socket: Socket
}
