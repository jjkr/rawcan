/// <reference path="../typings/main.d.ts"/>

export declare class CANWrap {
  constructor();
  bind(iface: string): number;
  send(id: number, buffer: Buffer): void;
  close(): void;
  setFilter(filter: number, mask: number): void;
  onSent(callback: (err: number) => void): void;
  onMessage(callback: (id: number, message: Buffer) => void): void;
  onError(callback: (err: number) => void): void;
  ref(): void;
  unref(): void;
}
