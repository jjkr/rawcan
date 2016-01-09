var rawcan = require('bindings')('rawcan');
rawcan.newSocket = function(iface) { return new rawcan.RawCanSocket(iface); };
module.exports = rawcan;
