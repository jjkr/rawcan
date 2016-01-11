'use strict';

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) defineProperties(Constructor.prototype, protoProps); if (staticProps) defineProperties(Constructor, staticProps); return Constructor; }; }();

Object.defineProperty(exports, "__esModule", {
  value: true
});

var _events = require('events');

var _events2 = _interopRequireDefault(_events);

var _bindings = require('bindings');

var _bindings2 = _interopRequireDefault(_bindings);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _possibleConstructorReturn(self, call) { if (!self) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return call && (typeof call === "object" || typeof call === "function") ? call : self; }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function, not " + typeof superClass); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } }); if (superClass) Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass; }

var CANWrap = (0, _bindings2.default)('can_wrap').CANWrap;

var Socket = function (_EventEmitter) {
  _inherits(Socket, _EventEmitter);

  function Socket() {
    _classCallCheck(this, Socket);

    var _this = _possibleConstructorReturn(this, Object.getPrototypeOf(Socket).call(this));

    _this._handle = new CANWrap();
    _this._receiving = false;
    _this._sendQueue = [];
    return _this;
  }

  _createClass(Socket, [{
    key: 'bind',
    value: function bind(iface) {
      if (this._receiving) {
        throw new Error('Socket is already bound');
      }

      var err = this._handle.bind(iface);
      if (err != 0) {
        throw new Error('failed to bind: ' + err);
      }

      //this._handle.recvStart();
      this._receiving = true;

      return this;
    }
  }, {
    key: 'send',
    value: function send(id, buffer, callback) {
      var sending = this._sendQueue.length > 0;
      this._sendQueue.push([id, buffer, callback]);

      if (typeof buffer === 'string') {
        buffer = new Buffer(buffer);
      } else if (!(buffer instanceof Buffer)) {
        throw TypeError('Second argument must be a Buffer or String');
      }

      // normalize callback so it is either a function or undefined
      if (typeof callback !== 'function') {
        callback = undefined;
      }

      if (!sending) {}
    }
  }, {
    key: '_onSent',
    value: function _onSent(err) {}
  }]);

  return Socket;
}(_events2.default);

exports.default = {
  Socket: Socket
};