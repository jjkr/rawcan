var rawcan = require('..');
//import rawcan from '..';

var sock = new rawcan.RawCanSocket("vcan0");
sock.setFilter(2, 3);
console.log('sending');
console.log(rawcan);
