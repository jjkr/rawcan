import {expect} from 'chai';
import can from '..';

describe('can Socket', () => {
  describe('constructor', () => {
    it('constructs default', () => {
      const sock = new can.Socket();
    });
    it('constructs with interface', () => {
      const sock = new can.Socket('vcan0');
    });
  });

  describe('bind', () => {
    it('binds to vcan0', () => {
      const sock = new can.Socket();
      sock.bind('vcan0');
    });
  });

  describe('send', () => {
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
    it('throws on bad buffer', () => {
      class Foo {}
      const sock = new can.Socket();
      sock.bind('vcan0');
      expect(() => {sock.send(0x34, new Foo())}).to.throw(TypeError);
    });
    it('sends two frames', () => {
      const sock = new can.Socket();
      sock.bind('vcan0');
      sock.send(0x34, new Buffer([0x12, 0x34]));
      sock.send(0x34, new Buffer([0x56, 0x78]));
    });
    it('triggers callback', () => {
      const sock = new can.Socket();
      sock.bind('vcan0');
      sock.send(0x34, 'asdf', (err) => {});
    });
  });
});
