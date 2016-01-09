var fs = require('fs');
var rawcan = require('..');

var sock = rawcan.newSocket('vcan0');
sock.setFilter(0x23, 0x43);
