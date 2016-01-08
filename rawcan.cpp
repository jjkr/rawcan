#include <nan.h>
#include <node.h>

using v8::Local;
using v8::Object;

namespace rawcan
{
/**
 * A basic socket to send and receive frames on a CAN bus
 */
class RawCanSocket : public node::ObjectWrap
{
public:
    static void init(v8::Local<v8::Object> exports)
    {
    }

private:
    RawCanSocket(v8::Isolate* isolate,
                 const v8::Local<v8::String>& iface) noexcept;

    const int m_socket;
};

void initModule(Local<Object> exports)
{
    RawCanSocket::init(exports);
}

NODE_MODULE(rawcan, initModule)
}
