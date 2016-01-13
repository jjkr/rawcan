var __extends = (this && this.__extends) || function (d, b) {
    for (var p in b) if (b.hasOwnProperty(p)) d[p] = b[p];
    function __() { this.constructor = d; }
    d.prototype = b === null ? Object.create(b) : (__.prototype = b.prototype, new __());
};
var events_1 = require('events');
var can_wrap_1 = require('./can_wrap');
var Socket = (function (_super) {
    __extends(Socket, _super);
    function Socket(iface) {
        var _this = this;
        _super.call(this);
        this._handle = new can_wrap_1.CANWrap();
        this._handle.onSent(function (err) { _this._onSent(err); });
    }
    Socket.prototype._onSent = function (err) {
    };
    return Socket;
})(events_1.EventEmitter);
exports.Socket = Socket;
