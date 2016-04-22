/// <library path="../typings/main.d.ts"/>
import {EventEmitter} from 'events';
/// <reference path="./can_wrap.d.ts"/>
import {CANWrap} from './can_wrap';

interface SendRequest {
  id: number;
  buffer: Buffer;
  callback: (err: number) => void;
}

export class Socket extends EventEmitter {
  private _handle: CANWrap;
  private _bound: boolean;
  private _sendQueue: SendRequest[];

  constructor(iface?: string) {
    super();
    this._handle = new CANWrap();
    this._handle.onSent((err) => { this._onSent(err); });
    this._handle.onMessage(
        (id, buffer) => { this.emit('message', id, buffer); });
    this._handle.onError((err) => { this.emit('error', err); });
    this._sendQueue = [];
    this._bound = false;
    if (iface) {
      this.bind(iface);
    }
  }

  bind(iface: string): Socket {
    if (this._bound) {
      throw new Error('Socket is already bound');
    }

    this._healthCheck();

    const err = this._handle.bind(iface);
    if (err != 0) {
      throw new Error('Failed to bind: ' + err);
    }

    this._bound = true;
    return this;
  }

  send(id: number, buffer: string | Buffer | number[],
       callback?: (err: number) => void): void {
    if (!(buffer instanceof Buffer)) {
      buffer = new Buffer(<any>buffer);
    }
    const castedBuffer = <Buffer>buffer;

    id = id >>> 0;
    this._healthCheck();
    const sending = this._sendQueue.length > 0;
    this._sendQueue.push({id: id, buffer: castedBuffer, callback: callback});

    if (!sending) {
      this._handle.send(id, castedBuffer);
    }
  }

  setFilter(filter: number, mask: number): void {
    this._healthCheck();
    this._handle.setFilter(filter >>> 0, mask);
  }

  close(): void {
    this._healthCheck();
    this._handle.close();
    this._handle = undefined;
    this.emit('close');
  }

  ref(): void {
    this._healthCheck();
    this._handle.ref();
  }

  unref(): void {
    this._healthCheck();
    this._handle.unref();
  }

  _onSent(err: number): void {
    const sent = this._sendQueue[0];
    if (sent.callback) {
      sent.callback(err);
    }
    this._sendQueue.shift();
    const next = this._sendQueue[0];
    if (next) {
      this._handle.send(next.id, next.buffer);
    }
  }

  _healthCheck(): void {
    if (!this._handle) {
      throw new Error('Not running');
    }
  }
}

export function createSocket(iface?: string) { return new Socket(iface); }

export const EFF_FLAG: number = 0x80000000;  // extended frame format
export const RTR_FLAG: number = 0x40000000;  // remote transmission request
export const ERR_FLAG: number = 0x20000000;  // error

export const SFF_MASK: number = 0x7FF;       // standard frame format - 11 bits
export const EFF_MASK: number = 0x1FFFFFFF;  // extended frame format - 29 bits
export const ERR_MASK: number = 0x1FFFFFFF;
