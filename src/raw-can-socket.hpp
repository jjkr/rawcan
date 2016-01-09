#pragma once
#include <nan.h>
#include <memory>

namespace rawcan
{
/**
 * A basic socket to send and receive frames on a CAN bus
 */
class RawCanSocket : public Nan::ObjectWrap
{
public:
    static NAN_MODULE_INIT(init);

private:
    explicit RawCanSocket(const char* iface);
    ~RawCanSocket();

    int getError() const { return m_error; }

    static NAN_METHOD(construct);
    static NAN_METHOD(setFilter);
    static NAN_METHOD(onFrame);
    static NAN_METHOD(send);
    static NAN_METHOD(stop);

    static Nan::Persistent<v8::Function> s_constructor;

    const int m_socket;
    std::unique_ptr<uv_poll_t> m_uvPoll;
    int m_error = 0;
    int m_pollEvents = 0;
};
}
