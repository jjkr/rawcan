#include "raw-can-socket.hpp"
#include <node.h>

using v8::Local;
using v8::Object;

namespace rawcan
{
void initModule(Local<Object> exports)
{
    RawCanSocket::init(exports);
}

NODE_MODULE(rawcan, initModule)
}
