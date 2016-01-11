# rawcan
[![Build Status](https://travis-ci.org/jjkr/rawcan.svg?branch=master)](https://travis-ci.org/jjkr/rawcan)

Lightweight Node.js bindings for raw CAN sockets on Linux.

    import can from 'rawcan';

    const server = can.createSocket('vcan0');

    server.on('error', (err) => { console.log('server error: ' + err); });
    server.on('message', (id, buffer) => {
      console.log('server got [' + id.toString(16) + '] ' + buffer.toString('hex'));
    });
