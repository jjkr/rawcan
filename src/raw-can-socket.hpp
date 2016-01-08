#pragma once
#include <node.h>
#include <node_object_wrap.h>

namespace rawcan
{
class RawCanSocket : public node::ObjectWrap
{
public:
    static void init(v8::Local<v8::Object> exports);

private:
    RawCanSocket(v8::Isolate* isolate,
                 const v8::Local<v8::String>& iface) noexcept;

    int m_socket;
};
}
