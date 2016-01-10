#include "raw-can-socket.hpp"
#include <node.h>
#include <uv.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <fcntl.h>
#include <unistd.h>
#include <cassert>
#include <memory>
#include <stdexcept>
#include <string>

using Nan::SetPrototypeMethod;
using v8::Local;
using v8::Function;
using v8::FunctionTemplate;
using std::make_unique;
using std::runtime_error;
using std::string;
using namespace std::string_literals;

namespace
{
string systemErrorString(int errorCode)
{
    return uv_err_name(errorCode) + ": "s + uv_strerror(errorCode);
}
}

namespace rawcan
{
Nan::Persistent<Function> RawCanSocket::s_constructor;

NAN_MODULE_INIT(RawCanSocket::init)
{
    Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(construct);
    tpl->SetClassName(Nan::New("RawCanSocket").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    SetPrototypeMethod(tpl, "setFilter", setFilter);
    //SetPrototypeMethod(tpl, "onFrame", onFrame);
    //SetPrototypeMethod(tpl, "send", send);
    //SetPrototypeMethod(tpl, "stop", stop);

    s_constructor.Reset(Nan::GetFunction(tpl).ToLocalChecked());
    Nan::Set(target, Nan::New("RawCanSocket").ToLocalChecked(),
             Nan::GetFunction(tpl).ToLocalChecked());
}

NAN_METHOD(RawCanSocket::construct)
{
    if (!info.IsConstructCall())
    {
        return Nan::ThrowError("Must be called with new");
    }
    if (info.Length() != 1)
    {
        return Nan::ThrowError("Too few arguments");
    }
    if (!info[0]->IsString())
    {
        return Nan::ThrowTypeError("First argument must be a string");
    }

    Nan::Utf8String iface(info[0]->ToString());
    auto* sock = new RawCanSocket(*iface);

    auto err = sock->getError();
    if (err < 0)
    {
        auto errMsg = systemErrorString(err);
        return Nan::ThrowError(errMsg.c_str());
    }

    sock->Wrap(info.This());
    info.GetReturnValue().Set(info.This());
}

RawCanSocket::RawCanSocket(const char* iface)
    : m_socket(socket(PF_CAN, SOCK_RAW, CAN_RAW)),
      m_uvPoll(make_unique<uv_poll_t>())
{
    if (m_socket < 0)
    {
        m_error = m_socket;
        return;
    }

    // set nonblocking mode
    auto flags = fcntl(m_socket, F_GETFL, 0);
    fcntl(m_socket, F_SETFL, flags | O_NONBLOCK);

    // set interface
    auto canAddr = sockaddr_can();
    auto ifr = ifreq();
    strcpy(ifr.ifr_name, iface);
    m_error = ioctl(m_socket, SIOCGIFINDEX, &ifr);
    if (m_error < 0)
    {
        goto on_error;
    }

    canAddr.can_family = AF_CAN;
    canAddr.can_ifindex = ifr.ifr_ifindex;

    // bind
    m_error = bind(m_socket, reinterpret_cast<struct sockaddr*>(&canAddr),
                   sizeof(canAddr));
    if (m_error < 0)
    {
        goto on_error;
    }

    uv_poll_init_socket(uv_default_loop(), m_uvPoll.get(), m_socket);
    m_uvPoll->data = this;

    return;

on_error:
    close(m_socket);
}

RawCanSocket::~RawCanSocket()
{
    if (!m_error)
    {
        uv_poll_stop(m_uvPoll.get());
        // The memory for the poll handle (uv_poll_t) needs to stick around for
        // the next run of the event loop so it can be closed properly. The
        // memory gets deleted in the close callback
        auto* handle = reinterpret_cast<uv_handle_t*>(m_uvPoll.release());
        uv_close(handle, [](auto* handle) {
            auto* pollHandle = reinterpret_cast<uv_poll_t*>(handle);
            delete pollHandle;
        });
        close(m_socket);
    }
}

NAN_METHOD(RawCanSocket::setFilter)
{
    RawCanSocket* socket = ObjectWrap::Unwrap<RawCanSocket>(info.Holder());

    if (info.Length() != 2)
    {
        return Nan::ThrowError("Wrong number of arguments");
    }
    if (!info[0]->IsNumber() || !info[1]->IsNumber())
    {
        return Nan::ThrowTypeError("Arguments must be numbers");
    }

    auto mask = Nan::To<uint32_t>(info[0]).FromJust();
    auto filter = Nan::To<uint32_t>(info[1]).FromJust();

    struct can_filter cf[1];
    cf[0].can_id = filter;
    cf[0].can_mask = mask;
    setsockopt(socket->m_socket, SOL_CAN_RAW, CAN_RAW_FILTER, &cf, sizeof(cf));
}
}
