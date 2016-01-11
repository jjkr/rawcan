#include "can_wrap.hpp"
#include <uv.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/can/raw.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <node.h>
#include <algorithm>
#include <cassert>
#include <cstdint>

// TODO JJK
#include <iostream>
using namespace std;

using Nan::Callback;
using v8::Local;
using v8::Function;
using v8::FunctionTemplate;
using v8::Value;
using std::begin;
using std::copy_n;

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
    SetPrototypeMethod(tpl, "close", Close);
    SetPrototypeMethod(tpl, "setFilter", SetFilter);
    SetPrototypeMethod(tpl, "onSent", OnSent);
    SetPrototypeMethod(tpl, "onMessage", OnMessage);
    SetPrototypeMethod(tpl, "onError", OnError);
    SetPrototypeMethod(tpl, "ref", Ref);
    SetPrototypeMethod(tpl, "unref", UnRef);

    s_constructor.Reset(Nan::GetFunction(tpl).ToLocalChecked());
    Nan::Set(target, Nan::New("CANWrap").ToLocalChecked(),
             Nan::GetFunction(tpl).ToLocalChecked());
}

CANWrap::CANWrap()
    : m_socket(socket(PF_CAN, SOCK_RAW, CAN_RAW)), m_uvHandle(new uv_poll_t)
{
    assert(m_socket);
    cout << "CANWrap::CANWrap" << endl;
    // set nonblocking mode
    int flags = fcntl(m_socket, F_GETFL, 0);
    fcntl(m_socket, F_SETFL, flags | O_NONBLOCK);

    uv_poll_init_socket(uv_default_loop(), m_uvHandle, m_socket);
    m_uvHandle->data = this;
}

CANWrap::~CANWrap()
{
    cout << "CANWrap::~CANWrap" << endl;
    doClose();
    delete m_uvHandle;
}

NAN_METHOD(CANWrap::New)
{
    cout << "CANWrap::New" << endl;
    assert(info.IsConstructCall());
    auto* can = new CANWrap();
    can->Wrap(info.This());
}

NAN_METHOD(CANWrap::Bind)
{
    cout << "CANWrap::Bind" << endl;
    assert(1 == info.Length());
    CANWrap* self = ObjectWrap::Unwrap<CANWrap>(info.Holder());

    Nan::Utf8String iface(info[0]->ToString());

    auto ifr = ifreq();
    strcpy(ifr.ifr_name, *iface);
    auto err = ioctl(self->m_socket, SIOCGIFINDEX, &ifr);

    if (err == 0)
    {
        auto canAddr = sockaddr_can();
        canAddr.can_family = AF_CAN;
        canAddr.can_ifindex = ifr.ifr_ifindex;

        err = bind(self->m_socket, reinterpret_cast<struct sockaddr*>(&canAddr),
                   sizeof(canAddr));
    }

    self->m_pollEvents |= UV_READABLE;
    self->doPoll();

    info.GetReturnValue().Set(err);
    cout << "CANWrap::Bind done" << endl;
}

NAN_METHOD(CANWrap::Send)
{
    cout << "CANWrap::Send" << endl;
    // send(id, buffer)
    assert(2 == info.Length());
    assert(info[0]->IsUint32());
    assert(node::Buffer::HasInstance(info[1]));

    auto* self = ObjectWrap::Unwrap<CANWrap>(info.Holder());
    assert(self);
    assert(!self->m_closed);

    auto& sendBuffer = self->m_sendBuffer;
    auto id = Nan::To<uint32_t>(info[0]).FromJust();
    sendBuffer.can_id = id;

    auto length = node::Buffer::Length(info[1]);
    sendBuffer.can_dlc = length;
    copy_n(node::Buffer::Data(info[1]), length, begin(sendBuffer.data));

    self->m_pollEvents |= UV_WRITABLE;
    self->doPoll();
}

NAN_METHOD(CANWrap::Close)
{
    cout << "CANWrap::Close" << endl;
    auto* self = ObjectWrap::Unwrap<CANWrap>(info.Holder());
    assert(self);

    self->doClose();
}

NAN_METHOD(CANWrap::SetFilter)
{
    cout << "CANWrap::SetFilter" << endl;
    // setFilter(filter, mask)
    assert(2 == info.Length());
    assert(info[0]->IsUint32());
    assert(info[1]->IsUint32());

    auto filter = Nan::To<uint32_t>(info[0]).FromJust();
    auto mask = Nan::To<uint32_t>(info[1]).FromJust();

    auto* self = ObjectWrap::Unwrap<CANWrap>(info.Holder());
    assert(self);

    struct can_filter cf[1];
    cf[0].can_id = filter;
    cf[0].can_mask = mask;
    setsockopt(self->m_socket, SOL_CAN_RAW, CAN_RAW_FILTER, &cf, sizeof(cf));
}

NAN_METHOD(CANWrap::OnSent)
{
    cout << "CANWrap::OnSent" << endl;
    // onSent(callback)
    assert(1 == info.Length());
    assert(info[0]->IsFunction());

    auto* self = ObjectWrap::Unwrap<CANWrap>(info.Holder());
    assert(self);

    self->m_sentCallback.SetFunction(info[0].As<v8::Function>());
}

NAN_METHOD(CANWrap::OnMessage)
{
    // onMessage(callback)
    assert(1 == info.Length());
    assert(info[0]->IsFunction());

    auto* self = ObjectWrap::Unwrap<CANWrap>(info.Holder());
    assert(self);

    self->m_messageCallback.SetFunction(info[0].As<v8::Function>());
}

NAN_METHOD(CANWrap::OnError)
{
    // onMessage(callback)
    assert(1 == info.Length());
    assert(info[0]->IsFunction());

    auto* self = ObjectWrap::Unwrap<CANWrap>(info.Holder());
    assert(self);

    self->m_errorCallback.SetFunction(info[0].As<v8::Function>());
}

NAN_METHOD(CANWrap::Ref)
{
    auto* self = ObjectWrap::Unwrap<CANWrap>(info.Holder());
    assert(self);

    uv_ref(reinterpret_cast<uv_handle_t*>(self->m_uvHandle));
}

NAN_METHOD(CANWrap::UnRef)
{
    auto* self = ObjectWrap::Unwrap<CANWrap>(info.Holder());
    assert(self);

    uv_unref(reinterpret_cast<uv_handle_t*>(self->m_uvHandle));
}

void CANWrap::uvPollCallback(uv_poll_t* pollHandle, int status,
                             int events)
{
    auto* self = static_cast<CANWrap*>(pollHandle->data);
    assert(self);
    self->pollCallback(status, events);
}

void CANWrap::pollCallback(int status, int events)
{
    cout << "CANWrap::pollCallback(" << status << ", " << events << ")" << endl;
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
            else
            {
                callErrorCallback(err);
            }
        }
        else if (events & UV_READABLE)
        {
            const int err = doRecv();
            if (err < 0)
            {
                callErrorCallback(errno);
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
        callErrorCallback(status);
    }
    cout << "CANWrap::pollCallback done" << endl;
}

int CANWrap::doPoll()
{
    cout << "CANWrap::doPoll" << endl;
    if (m_closed)
    {
        return -1;
    }

    if (m_pollEvents)
    {
        return uv_poll_start(m_uvHandle, m_pollEvents, uvPollCallback);
    }
    else
    {
        return uv_poll_stop(m_uvHandle);
    }
    cout << "CANWrap::doPoll done" << endl;
}

int CANWrap::doSend()
{
    cout << "CANWrap::doSend" << endl;
    return ::send(m_socket, &m_sendBuffer, sizeof(m_sendBuffer), 0);
}

int CANWrap::doRecv()
{
    cout << "CANWrap::doRecv" << endl;
    return ::recv(m_socket, &m_recvBuffer, sizeof(m_recvBuffer), 0);
}

void CANWrap::doClose()
{
    if (!m_closed)
    {
        uv_poll_stop(m_uvHandle);
        uv_close(reinterpret_cast<uv_handle_t*>(m_uvHandle),
                 [](uv_handle_t* handle) {
                     auto* poll = reinterpret_cast<uv_poll_t*>(handle);
                     //delete poll;
                 });
        m_uvHandle = nullptr;
        m_closed = true;
    }
}

void CANWrap::callErrorCallback(int err)
{
    cout << "CANWrap::callErrorCallback" << endl;
    if (!m_errorCallback.IsEmpty())
    {
        Nan::HandleScope scope;
        Local<Value> argv[1] = {Nan::New(err)};
        m_errorCallback.Call(1, argv);
    }
}
}
