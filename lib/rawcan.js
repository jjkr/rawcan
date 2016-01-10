import EventEmitter from 'events';
import bindings from 'bindings';

const CANWrap = bindings('can_wrap').CANWrap;

const BIND_STATE_UNBOUND = 0;
const BIND_STATE_BINDING = 1;
const BIND_STATE_BOUND = 2;

class Socket extends EventEmitter {
  constructor() {
    super();
    this._handle = new CANWrap();
    this._receiving = false;
    this._bindState = BIND_STATE_UNBOUND;
  }
}

export default {
  Socket: Socket
}
