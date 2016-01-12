const can = require('..');

const server = can.createSocket('vcan0');
server.on('message', (id, buffer) => {
  console.log('[' + id.toString(16) + '] ' + buffer.toString('hex'));
  server.close();
});

var p = new Promise(function(resolve, reject) {
  const client = can.createSocket('vcan0');
  client.send(0x22, new Buffer([0xDE, 0xAD, 0xBE, 0xEF]), function() {
    client.close();
    resolve();
  });
});

p.then(function() {
  console.log('garbage collecting');
  //global.gc()
});
;

