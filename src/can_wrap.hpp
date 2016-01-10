#pragma once
#include <nan.h>
#include <v8.h>

namespace rawcan
{
class CANWrap : public Nan::ObjectWrap
{
public:
    static NAN_MODULE_INIT(Initialize);

private:
    CANWrap() noexcept;

    static NAN_METHOD(New);
    static NAN_METHOD(Bind);

    static Nan::Persistent<v8::Function> s_constructor;

    const int m_socket;
    uv_poll_t m_uvHandle;
};

NODE_MODULE(CANWrap, CANWrap::Initialize);
}
