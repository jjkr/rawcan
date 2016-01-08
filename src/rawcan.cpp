#include <node.h>

using v8::Local;
using v8::Object;

namespace rawcan
{
class Channel
{
};

void init(Local<Object> exports)
{
    //TextBuffer::init(exports);
}

NODE_MODULE(rawcan, init)
}
