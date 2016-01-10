import EventEmitter from 'events';
const CANWrap = require('bindings')('can_wrap');

const BIND_STATE_UNBOUND = 0;
const BIND_STATE_BINDING = 1;
const BIND_STATE_BOUND = 2;

export class Socket extends EventEmitter {
  constructor() {
    super();
    this._handle = new CANWrap();
    this._receiving = false;
    this._bindState = BIND_STATE_UNBOUND;
  }
}
