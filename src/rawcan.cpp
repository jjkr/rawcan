#include "raw-can-socket.hpp"
#include <nan.h>
#include <node.h>

namespace rawcan
{
NAN_MODULE_INIT(initModule)
{
    RawCanSocket::init(target);
}

NODE_MODULE(rawcan, initModule)
}
