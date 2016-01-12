const can = require('..');

const server = can.createSocket('vcan0');
server.on('message', (id, buffer) => {
  var bufStr = '';
  for (b of buffer) {
    bufStr += b.toString(16);
    bufStr += ' ';
  }
  console.log('[' + id.toString(16) + '] ' + bufStr);
});
