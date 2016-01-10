import EventEmitter from 'events';
import bindings from 'bindings';

const CANWrap = bindings('can_wrap').CANWrap;

class Socket extends EventEmitter {
  constructor() {
    super();
    this._handle = new CANWrap();
    this._receiving = false;
  }

  bind(iface) {
    if (this._receiving) {
      throw new Error('Socket is already bound');
    }

    const err = this._handle.bind(iface);
    if(err != 0) {
      throw new Error('failed to bind: ' + err);
    }
  }
}

export default {
  Socket: Socket
}
