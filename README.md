# rawcan
[![Build Status](https://travis-ci.org/jjkr/rawcan.svg?branch=master)](https://travis-ci.org/jjkr/rawcan)

Lightweight asynchronous Node.js bindings for SocketCAN. SocketCAN is a socket based implementation of the CAN bus protocol for the Linux kernel, developed primarily by VW.

```javascript
import can from 'rawcan';

const socket = can.createSocket('vcan0');

socket.on('error', (err) => { console.log('socket error: ' + err); });
socket.on('message', (id, buffer) => {
  console.log('received frame [' + id.toString(16) + '] ' + buffer.toString('hex'));
});

socket.send(CAN_EFF_MASK | 0x23c89f, 'hello');
```

How is this different from node-can?
------------------------------------

There is another node package for SocketCAN called [node-can](https://github.com/sebi2k1/node-can) (socketcan in npm). The main differences are it does not support asynchronous sending or TypeScript bindings, which this package does. It also has some messaging features, while this package is simply a thin wrapper around raw CAN sockets.
