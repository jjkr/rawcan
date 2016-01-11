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
#include <utility>

using Nan::Callback;
using v8::Local;
using v8::Function;
using v8::FunctionTemplate;
using std::begin;
using std::copy_n;
using std::make_unique;

namespace rawcan
{
Nan::Persistent<Function> CANWrap::s_constructor;

NAN_MODULE_INIT(CANWrap::Initialize)
{
    Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("CANWrap").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    SetPrototypeMethod(tpl, "bind", Bind);
    SetPrototypeMethod(tpl, "send", Send);
    //SetPrototypeMethod(tpl, "setFilter", SetFilter);
    //SetPrototypeMethod(tpl, "onFrame", onFrame);
    //SetPrototypeMethod(tpl, "send", send);
    //SetPrototypeMethod(tpl, "stop", stop);

    s_constructor.Reset(Nan::GetFunction(tpl).ToLocalChecked());
    Nan::Set(target, Nan::New("CANWrap").ToLocalChecked(),
             Nan::GetFunction(tpl).ToLocalChecked());
}

CANWrap::CANWrap() noexcept : m_socket(socket(PF_CAN, SOCK_RAW, CAN_RAW))
{
    // set nonblocking mode
    auto flags = fcntl(m_socket, F_GETFL, 0);
    fcntl(m_socket, F_SETFL, flags | O_NONBLOCK);

    uv_poll_init_socket(uv_default_loop(), &m_uvHandle, m_socket);
    m_uvHandle.data = this;
}

NAN_METHOD(CANWrap::New)
{
    assert(info.IsConstructCall());
    auto* can = new CANWrap();
    can->Wrap(info.This());
}

NAN_METHOD(CANWrap::Bind)
{
    assert(1 == info.Length());
    CANWrap* obj = ObjectWrap::Unwrap<CANWrap>(info.Holder());

    Nan::Utf8String iface(info[0]->ToString());

    auto ifr = ifreq();
    strcpy(ifr.ifr_name, *iface);
    auto err = ioctl(obj->m_socket, SIOCGIFINDEX, &ifr);

    if (err == 0)
    {
        auto canAddr = sockaddr_can();
        canAddr.can_family = AF_CAN;
        canAddr.can_ifindex = ifr.ifr_ifindex;

        err = bind(obj->m_socket, reinterpret_cast<struct sockaddr*>(&canAddr),
                   sizeof(canAddr));
    }

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

    auto& sendBuffer = self->m_sendBuffer;
    auto id = Nan::To<uint32_t>(info[0]).FromJust();
    sendBuffer.can_id = id;

    auto nodeBuffer = info[1];
    auto length = node::Buffer::Length(nodeBuffer);
    sendBuffer.can_dlc = length;
    std::copy_n(node::Buffer::Data(nodeBuffer), length, begin(sendBuffer.data));
}

NAN_METHOD(CANWrap::OnSent)
{
    // onSent(callback)
    assert(1 == info.Length());

    CANWrap* self = ObjectWrap::Unwrap<CANWrap>(info.Holder());
    assert(self);

    self->m_sentCallback = make_unique<Callback>(info[0].As<v8::Function>());
}

void CANWrap::uvPollCallback(uv_poll_t* pollHandle, int status,
                             int events) noexcept
{
    auto* self = static_cast<CANWrap*>(pollHandle->data);
    assert(self);
    self->pollCallback(status, events);
}

void CANWrap::pollCallback(int status, int events) noexcept
{
    if (status == 0)
    {
        if (events & UV_WRITABLE)
        {
            const auto err = doSend();
            m_pollEvents &= ~UV_WRITABLE;
            if (err < 0)
            {
                // TODO handle send error
            }

            doPoll();
        }
        else if (events & UV_READABLE)
        {
            const auto err = doRecv();
            if (err < 0)
            {
                // TODO handle send error
            }
        }
    }
    else
    {
        // TODO handle system error on poll()
    }
}

int CANWrap::doPoll() noexcept
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

int CANWrap::doSend() noexcept
{
    return ::send(m_socket, &m_sendBuffer, sizeof(m_sendBuffer), 0);
}

int CANWrap::doRecv() noexcept
{
    return ::recv(m_socket, &m_recvBuffer, sizeof(m_recvBuffer), 0);
}
}
