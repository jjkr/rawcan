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

  describe('iface', () => {
    it('returns the interface', () => {
      const iface = 'vcan0';
      const sock = new can.Socket(iface);
      expect(sock.iface).to.equal(iface);
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
    it('triggers callback', (done) => {
      const sock = new can.Socket();
      sock.bind('vcan0');
      sock.send(0x34, new Buffer([0xca, 0xfe]), (err) => {
        if (err) {
          throw Error('Error: ' + err);
        }
        done();
      });
    });
    it('sends two frames', (done) => {
      const sock = new can.Socket();
      sock.bind('vcan0');
      sock.send(0x34, new Buffer([0x12, 0x34]));
      sock.send(0x34, new Buffer([0x56, 0x78]), (err) => { done(); });
    });
  });

  describe('recv', () => {
    it('receives a message', (done) => {
      const server = new can.Socket('vcan0');
      server.on('message', (id, buffer) => { done(); });
      const client = new can.Socket('vcan0');
      client.send(0x34, new Buffer([0xDE, 0xAD, 0xBE, 0xEF]));
    });
  });
});
