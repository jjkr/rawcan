#include "can_wrap.hpp"
#include <uv.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/can/raw.h>
#include <fcntl.h>
#include <unistd.h>
#include <node.h>
#include <algorithm>
#include <cassert>
#include <cstdint>
#include <cstring>

using Nan::Callback;
using v8::Local;
using v8::Function;
using v8::FunctionTemplate;
using v8::Value;

namespace rawcan
{
Nan::Persistent<Function> CANWrap::s_constructor;

NAN_MODULE_INIT(CANWrap::Initialize)
{
    Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("CANWrap").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    SetPrototypeMethod(tpl, "onSent", OnSent);
    SetPrototypeMethod(tpl, "onMessage", OnMessage);
    SetPrototypeMethod(tpl, "bind", Bind);
    SetPrototypeMethod(tpl, "send", Send);
    SetPrototypeMethod(tpl, "close", Close);
    SetPrototypeMethod(tpl, "setFilter", SetFilter);
    SetPrototypeMethod(tpl, "ref", Ref);
    SetPrototypeMethod(tpl, "unref", UnRef);

    s_constructor.Reset(Nan::GetFunction(tpl).ToLocalChecked());
    Nan::Set(target, Nan::New("CANWrap").ToLocalChecked(),
             Nan::GetFunction(tpl).ToLocalChecked());
}

CANWrap::CANWrap()
    : m_socket(socket(PF_CAN, SOCK_RAW, CAN_RAW)), m_pollEvents(0)
{
    // set nonblocking mode
    int flags = fcntl(m_socket, F_GETFL, 0);
    fcntl(m_socket, F_SETFL, flags | O_NONBLOCK);

    uv_poll_init_socket(uv_default_loop(), &m_uvHandle, m_socket);
    m_uvHandle.data = this;
}

NAN_METHOD(CANWrap::New)
{
    assert(info.IsConstructCall());
    CANWrap* can = new CANWrap();
    can->Wrap(info.This());
}

NAN_METHOD(CANWrap::Bind)
{
    assert(1 == info.Length());
    CANWrap* self = ObjectWrap::Unwrap<CANWrap>(info.Holder());

    Nan::Utf8String iface(info[0]->ToString());

    ifreq ifr = ifreq();
    strcpy(ifr.ifr_name, *iface);
    int err = ioctl(self->m_socket, SIOCGIFINDEX, &ifr);

    if (err == 0)
    {
        sockaddr_can canAddr = sockaddr_can();
        canAddr.can_family = AF_CAN;
        canAddr.can_ifindex = ifr.ifr_ifindex;

        err = bind(self->m_socket, reinterpret_cast<struct sockaddr*>(&canAddr),
                   sizeof(canAddr));
    }

    self->m_pollEvents |= UV_READABLE;
    self->doPoll();

    info.GetReturnValue().Set(err);
}

NAN_METHOD(CANWrap::Send)
{
    // send(id, buffer)
    assert(2 == info.Length());
    assert(info[0]->IsUint32());
    assert(node::Buffer::HasInstance(info[1]));

    CANWrap* self = ObjectWrap::Unwrap<CANWrap>(info.Holder());
    assert(self);

    can_frame& sendBuffer = self->m_sendBuffer;
    uint32_t id = Nan::To<uint32_t>(info[0]).FromJust();
    sendBuffer.can_id = id;

    size_t length = node::Buffer::Length(info[1]);
    sendBuffer.can_dlc = length;
    memcpy(sendBuffer.data, node::Buffer::Data(info[1]), length);

    self->m_pollEvents |= UV_WRITABLE;
    self->doPoll();
}

NAN_METHOD(CANWrap::Close)
{
    CANWrap* self = ObjectWrap::Unwrap<CANWrap>(info.Holder());
    assert(self);

    uv_poll_stop(&self->m_uvHandle);
    uv_close(reinterpret_cast<uv_handle_t*>(&self->m_uvHandle),
             uvCloseCallback);
}

NAN_METHOD(CANWrap::SetFilter)
{
    // setFilter(filter, mask)
    assert(2 == info.Length());
    assert(info[0]->IsUint32());
    assert(info[1]->IsUint32());

    uint32_t filter = Nan::To<uint32_t>(info[0]).FromJust();
    uint32_t mask = Nan::To<uint32_t>(info[1]).FromJust();

    CANWrap* self = ObjectWrap::Unwrap<CANWrap>(info.Holder());
    assert(self);

    struct can_filter cf[1];
    cf[0].can_id = filter;
    cf[0].can_mask = mask;
    setsockopt(self->m_socket, SOL_CAN_RAW, CAN_RAW_FILTER, &cf, sizeof(cf));
}

NAN_METHOD(CANWrap::OnSent)
{
    // onSent(callback)
    assert(1 == info.Length());

    CANWrap* self = ObjectWrap::Unwrap<CANWrap>(info.Holder());
    assert(self);

    self->m_sentCallback.SetFunction(info[0].As<v8::Function>());
}

NAN_METHOD(CANWrap::OnMessage)
{
    // onMessage(callback)
    assert(1 == info.Length());

    CANWrap* self = ObjectWrap::Unwrap<CANWrap>(info.Holder());
    assert(self);

    self->m_messageCallback.SetFunction(info[0].As<v8::Function>());
}

NAN_METHOD(CANWrap::Ref)
{
    CANWrap* self = ObjectWrap::Unwrap<CANWrap>(info.Holder());
    assert(self);

    uv_ref(reinterpret_cast<uv_handle_t*>(&self->m_uvHandle));
}

NAN_METHOD(CANWrap::UnRef)
{
    CANWrap* self = ObjectWrap::Unwrap<CANWrap>(info.Holder());
    assert(self);

    uv_unref(reinterpret_cast<uv_handle_t*>(&self->m_uvHandle));
}

void CANWrap::uvCloseCallback(uv_handle_t* handle)
{
    CANWrap* can = reinterpret_cast<CANWrap*>(handle->data);
    close(can->m_socket);
}

void CANWrap::uvPollCallback(uv_poll_t* pollHandle, int status,
                             int events)
{
    CANWrap* self = static_cast<CANWrap*>(pollHandle->data);
    assert(self);
    self->pollCallback(status, events);
}

void CANWrap::pollCallback(int status, int events)
{
    if (status == 0)
    {
        if (events & UV_WRITABLE)
        {
            const int err = doSend() < 0 ? errno : 0;

            m_pollEvents &= ~UV_WRITABLE;
            doPoll();
            if (!m_sentCallback.IsEmpty())
            {
                Nan::HandleScope scope;
                Local<Value> argv[1] = {Nan::New(err)};
                m_sentCallback.Call(1, argv);
            }
        }
        else if (events & UV_READABLE)
        {
            const int err = doRecv();
            if (err < 0)
            {
                // handle error
            }
            else if (!m_messageCallback.IsEmpty())
            {
                Nan::HandleScope scope;
                Local<Value> argv[] = {
                    Nan::New(m_recvBuffer.can_id),
                    Nan::NewBuffer(reinterpret_cast<char*>(&m_recvBuffer.data),
                                   m_recvBuffer.can_dlc)
                        .ToLocalChecked()};
                m_messageCallback.Call(2, argv);
            }
        }
    }
    else
    {
        // TODO handle system error on poll()
    }
}

int CANWrap::doPoll()
{
    if (m_pollEvents)
    {
        return uv_poll_start(&m_uvHandle, m_pollEvents, uvPollCallback);
    }
    else
    {
        return uv_poll_stop(&m_uvHandle);
    }
}

int CANWrap::doSend()
{
    return ::send(m_socket, &m_sendBuffer, sizeof(m_sendBuffer), 0);
}

int CANWrap::doRecv()
{
    return ::recv(m_socket, &m_recvBuffer, sizeof(m_recvBuffer), 0);
}
}
