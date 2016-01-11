#pragma once
#include <linux/can.h>
#include <nan.h>
#include <v8.h>

namespace rawcan
{
class CANWrap : public Nan::ObjectWrap
{
public:
    static NAN_MODULE_INIT(Initialize);

private:
    CANWrap();

    static NAN_METHOD(New);
    static NAN_METHOD(Bind);
    static NAN_METHOD(Send);
    static NAN_METHOD(Close);
    static NAN_METHOD(SetFilter);
    static NAN_METHOD(OnSent);
    static NAN_METHOD(OnMessage);
    static NAN_METHOD(Ref);
    static NAN_METHOD(UnRef);

    static void uvCloseCallback(uv_handle_t* handle);
    static void uvPollCallback(uv_poll_t* pollHandle, int status,
                               int events);
    void pollCallback(int status, int events);
    int doPoll();
    int doSend();
    int doRecv();

    static Nan::Persistent<v8::Function> s_constructor;

    Nan::Callback m_sentCallback;
    Nan::Callback m_messageCallback;

    const int m_socket;
    uv_poll_t m_uvHandle;

    can_frame m_recvBuffer;

    can_frame m_sendBuffer;
    int m_pollEvents;
};

NODE_MODULE(CANWrap, CANWrap::Initialize);
}
