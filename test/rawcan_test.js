import {expect} from 'chai';
import can from '..';

//import segvhandler from 'segfault-handler';
//segvhandler.registerHandler("crash.log");

describe('can Socket', () => {
  describe('constructor', () => {
    it('constructs default', () => {
      const sock = new can.Socket();
    });
    it('constructs with interface', () => {
      const sock = new can.Socket('vcan0');
    });
    it('createsSocket', () => {
      const sock = can.createSocket();
    });
  });

  describe('bind', () => {
    it('binds to vcan0', () => {
      const sock = new can.Socket();
      sock.bind('vcan0');
      sock.close();
    });
    it('fails to bind', () => {
      const sock = new can.Socket();
      expect(() => { sock.bind('bogus'); }).to.throw(Error);
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
    it('sends a buffer', (done) => {
      const sock = can.createSocket('vcan0');
      sock.send(0x34, new Buffer([0xDE, 0xAD, 0xBE, 0xEF]), () => {
        sock.close();
        done();
      });
    });
    it('sends a string', (done) => {
      const sock = can.createSocket();
      sock.bind('vcan0');
      sock.send(0x34, 'asdf', () => {
        sock.close();
        done();
      });
    });
    it('throws on bad buffer', () => {
      class Foo {}
      const sock = new can.Socket();
      sock.bind('vcan0');
      expect(() => {sock.send(0x34, new Foo())}).to.throw(TypeError);
      sock.close();
    });
    it('triggers callback', (done) => {
      const sock = new can.Socket();
      sock.bind('vcan0');
      sock.send(0x34, new Buffer([0xca, 0xfe]), (err) => {
        if (err) {
          throw Error('Error: ' + err);
        }
        sock.close();
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
      server.on('message', (id, buffer) => {
        server.close();
        done();
      });
      const client = new can.Socket('vcan0');
      client.send(86, 'hello');
    });
  });

  describe('setFilter', () => {
    it('filters frames', (done) => {
      const filteredId = 86;
      const server = new can.Socket('vcan0');
      server.setFilter(filteredId, 0xff);
      server.on('message', (id, buffer) => {
        server.close();
        expect(id).to.equal(filteredId);
        done();
      });

      const client = new can.Socket('vcan0');
      client.send(1, 'foo');
      client.send(99, 'bar');
      client.send(filteredId, 'hello');
      client.send(39, 'bar');
    });
  });
});
