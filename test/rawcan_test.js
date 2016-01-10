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
});
