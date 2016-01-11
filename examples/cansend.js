import can from '..';

const server = new can.Socket('vcan0');

server.unref();

server.on('message', (id, buffer) => {
  console.log('[' + id.toString(16) + '] ' + buffer.toString('hex'));
});
