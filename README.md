# rawcan
[![Build Status](https://travis-ci.org/jjkr/rawcan.svg?branch=master)](https://travis-ci.org/jjkr/rawcan)

Lightweight asynchronous Node.js bindings for SocketCAN. SocketCAN is a socket based implementation of the CAN bus protocol for the Linux kernel, developed primarily by VW.

```javascript
import * as can from 'rawcan';

const socket = can.createSocket('vcan0');

socket.on('error', err => { console.log('socket error: ' + err); });
socket.on('message', (id, buffer) => {
  console.log('received frame [' + id.toString(16) + '] ' + buffer.toString('hex'));
});

socket.send(can.EFF_FLAG | 0x23c89f, 'hello');
```

Installing
----------

This package is published to npm, so installing is as simple as:

```
$ npm install rawcan
```

Typescript bindings are included, so Typescript should just work.

Development
-----------

The workflow is mostly standard for a node addon. There is a native component, so a c++ compiler and make are required. In Ubuntu these dependencies can be installed by running the following in your shell

```
$ sudo aptitude install build-essential
```

After you have cloned the repo, run the following to install the npm dependencies and build the code:

```
$ npm install
$ npm run build
```

Running the tests requires a virtual CAN network interface called vcan0.

```
$ sudo modprobe vcan
$ sudo ip link add type vcan
$ sudo ip link set vcan0 up
```

Then the tests can be run with npm

```
$ npm test
```

How is this different from node-can?
------------------------------------

There is another node package for SocketCAN called [node-can](https://github.com/sebi2k1/node-can) (socketcan in npm). The main difference is that node-can is not evented- sends are synchronous and receives are done on a separate thread. By contrast, this package ties into the libuv based node event system, which uses epoll under the hood. node-can also has some messaging features, while this package is strictly a thin wrapper around raw CAN sockets.
