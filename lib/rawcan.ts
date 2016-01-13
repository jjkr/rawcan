/// <reference path="../typings/tsd.d.ts"/>
import {EventEmitter} from 'events';
/// <reference path="./can_wrap.d.ts"/>
import {CANWrap} from './can_wrap';

export class Socket extends EventEmitter {
  private _handle : CANWrap;

  constructor(iface: string) {
    super();
    this._handle = new CANWrap();
    this._handle.onSent((err) => { this._onSent(err); });
  }

  _onSent(err: number) {
    //const sent = this._sendQueue[0];
    //if (sent.callback) {
    //  sent.callback(err);
    //}
    //this._sendQueue.shift();
    //const next = this._sendQueue[0];
    //if (this._sendQueue.length > 0) {
    //  this._handle.send(next.id, next.buffer);
    //}
  }
}
