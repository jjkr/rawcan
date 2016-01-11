#pragma once
#include <linux/can.h>
#include <nan.h>
#include <v8.h>
#include <memory>

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
    static NAN_METHOD(Send);
    static NAN_METHOD(Close);
    static NAN_METHOD(OnSent);
    static NAN_METHOD(OnReceived);

    static void uvPollCallback(uv_poll_t* pollHandle, int status,
                               int events) noexcept;
    void pollCallback(int status, int events) noexcept;
    int doPoll() noexcept;
    int doSend() noexcept;
    int doRecv() noexcept;

    static Nan::Persistent<v8::Function> s_constructor;

    std::unique_ptr<Nan::Callback> m_sentCallback;
    std::unique_ptr<Nan::Callback> m_receivedCallback;

    const int m_socket;
    uv_poll_t m_uvHandle;

    can_frame m_recvBuffer;

    can_frame m_sendBuffer;
    bool m_sendPending = false;
    int m_pollEvents = 0;
};

NODE_MODULE(CANWrap, CANWrap::Initialize);
}
