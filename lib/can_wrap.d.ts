/// <reference path="../typings/tsd.d.ts"/>

export declare class CANWrap {
  constructor();
  bind(iface: string);
  send(id: number, buffer: Buffer);
  close();
  setFilter(filter: number, mask: number);
  onSent(callback: (err: number) => void);
  onMessage(callback: (id: number, message: Buffer) => void);
  onError(callback: (err: number) => void);
  ref();
  unref();
}
