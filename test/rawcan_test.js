import {expect} from 'chai';
import can from '..';

describe('can', () => {
  it('creates a socket', () => {
    const sock = new can.Socket();
  });
  it('binds to vcan0', () => {
    const sock = new can.Socket();
    sock.bind('vcan0');
  });
  it('sends a buffer', () => {
    const sock = new can.Socket();
    sock.bind('vcan0');
    sock.send(0x34, new Buffer([0xDE, 0xAD, 0xBE, 0xEF]));
  });
  it('sends a string', () => {
    const sock = new can.Socket();
    sock.bind('vcan0');
    sock.send(0x34, 'asdf');
  });
});
