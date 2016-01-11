'use strict';

var _createClass = function () { function defineProperties(target, props) { for (var i = 0; i < props.length; i++) { var descriptor = props[i]; descriptor.enumerable = descriptor.enumerable || false; descriptor.configurable = true; if ("value" in descriptor) descriptor.writable = true; Object.defineProperty(target, descriptor.key, descriptor); } } return function (Constructor, protoProps, staticProps) { if (protoProps) defineProperties(Constructor.prototype, protoProps); if (staticProps) defineProperties(Constructor, staticProps); return Constructor; }; }();

Object.defineProperty(exports, "__esModule", {
  value: true
});

var _events = require('events');

var _bindings = require('bindings');

var _bindings2 = _interopRequireDefault(_bindings);

function _interopRequireDefault(obj) { return obj && obj.__esModule ? obj : { default: obj }; }

function _classCallCheck(instance, Constructor) { if (!(instance instanceof Constructor)) { throw new TypeError("Cannot call a class as a function"); } }

function _possibleConstructorReturn(self, call) { if (!self) { throw new ReferenceError("this hasn't been initialised - super() hasn't been called"); } return call && (typeof call === "object" || typeof call === "function") ? call : self; }

function _inherits(subClass, superClass) { if (typeof superClass !== "function" && superClass !== null) { throw new TypeError("Super expression must either be null or a function, not " + typeof superClass); } subClass.prototype = Object.create(superClass && superClass.prototype, { constructor: { value: subClass, enumerable: false, writable: true, configurable: true } }); if (superClass) Object.setPrototypeOf ? Object.setPrototypeOf(subClass, superClass) : subClass.__proto__ = superClass; }

var CANWrap = (0, _bindings2.default)('can_wrap.node').CANWrap;

var Socket = function (_EventEmitter) {
  _inherits(Socket, _EventEmitter);

  function Socket(iface) {
    _classCallCheck(this, Socket);

    var _this = _possibleConstructorReturn(this, Object.getPrototypeOf(Socket).call(this));

    _this._handle = new CANWrap();
    _this._handle.owner = _this;
    _this._handle.onSent(function (err) {
      _this._onSent(err);
    });
    _this._handle.onMessage(function (id, buffer) {
      _this.emit('message', id, buffer);
    });
    _this._handle.onError(function (err) {
      _this.emit('error', err);
    });
    _this._bound = false;
    _this._sendQueue = [];
    if (typeof iface === 'string' || iface instanceof String) {
      _this.bind(iface);
    }
    return _this;
  }

  _createClass(Socket, [{
    key: 'bind',
    value: function bind(iface) {
      if (this._bound) {
        throw new Error('Socket is already bound');
      }

      this._healthCheck();

      var err = this._handle.bind(iface);
      if (err != 0) {
        throw new Error('Failed to bind: ' + err);
      }

      this._iface = iface;
      this._bound = true;
      return this;
    }
  }, {
    key: 'send',
    value: function send(id, buffer, callback) {
      if (typeof buffer === 'string') {
        buffer = new Buffer(buffer);
      } else if (!(buffer instanceof Buffer)) {
        throw TypeError('Second argument must be a Buffer or String');
      }

      // normalize callback so it is either a function or undefined
      if (typeof callback !== 'function') {
        callback = undefined;
      }

      this._healthCheck();

      var sending = this._sendQueue.length > 0;
      this._sendQueue.push({ id: id, buffer: buffer, callback: callback });

      if (!sending) {
        this._handle.send(id, buffer);
      }
    }
  }, {
    key: 'setFilter',
    value: function setFilter(filter, mask) {
      this._healthCheck();
      this._handle.setFilter(filter, mask);
    }
  }, {
    key: 'close',
    value: function close() {
      this._healthCheck();
      this._handle.close();
      this._handle = undefined;
      this.emit('close');
    }
  }, {
    key: 'ref',
    value: function ref() {
      this._healthCheck();
      this._handle.ref();
    }
  }, {
    key: 'unref',
    value: function unref() {
      this._healthCheck();
      this._handle.unref();
    }
  }, {
    key: '_onSent',
    value: function _onSent(err) {
      var sent = this._sendQueue[0];
      if (sent.callback) {
        sent.callback(err);
      }
      this._sendQueue.shift();
      var next = this._sendQueue[0];
      if (this._sendQueue.length > 0) {
        this._handle.send(next.id, next.buffer);
      }
    }
  }, {
    key: '_healthCheck',
    value: function _healthCheck() {
      if (!this._handle) {
        throw new Error('Not running');
      }
    }
  }, {
    key: 'iface',
    get: function get() {
      return this._iface;
    }
  }]);

  return Socket;
}(_events.EventEmitter);

exports.default = {
  EFF_FLAG: 0x80000000, // extended frame format
  RTR_FLAG: 0x40000000, // remote transmission request
  ERR_FLAG: 0x20000000, // error

  SFF_MASK: 0x7FF, // standard frame format - 11 bits
  EFF_MASK: 0x1FFFFFFF, // extended frame format - 29 bits
  ERR_MASK: 0x1FFFFFFF,

  Socket: Socket,
  createSocket: function createSocket(iface) {
    return new Socket(iface);
  }
};