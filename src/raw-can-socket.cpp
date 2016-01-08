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
#include <stdexcept>
#include <string>

using Nan::SetPrototypeMethod;
using v8::Local;
using v8::Function;
using v8::FunctionTemplate;
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

    //SetPrototypeMethod(tpl, "setFilter", setFilter);
    //SetPrototypeMethod(tpl, "onFrame", onFrame);
    //SetPrototypeMethod(tpl, "send", send);
    //SetPrototypeMethod(tpl, "stop", stop);

    s_constructor.Reset(Nan::GetFunction(tpl).ToLocalChecked());
    Nan::Set(target, Nan::New("MyObject").ToLocalChecked(),
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
    sock->Wrap(info.This());

    info.GetReturnValue().Set(info.This());
}

RawCanSocket::RawCanSocket(const char* iface)
    : m_socket(socket(PF_CAN, SOCK_RAW, CAN_RAW))
{
    if (m_socket < 0)
    {
        throw runtime_error(systemErrorString(m_socket));
    }

    // set nonblocking mode
    auto flags = fcntl(m_socket, F_GETFL, 0);
    fcntl(m_socket, F_SETFL, flags | O_NONBLOCK);

    // set interface
    auto canAddr = sockaddr_can();
    auto ifr = ifreq();
    strcpy(ifr.ifr_name, iface);
    ioctl(m_socket, SIOCGIFINDEX, &ifr);
    canAddr.can_family = AF_CAN;
    canAddr.can_ifindex = ifr.ifr_ifindex;

    // bind
    auto err = bind(m_socket, reinterpret_cast<struct sockaddr*>(&canAddr),
                    sizeof(canAddr));
    if (err < 0)
    {
        close(m_socket);
        throw runtime_error(systemErrorString(err));
    }

    uv_poll_init_socket(uv_default_loop(), m_uvPoll.get(), m_socket);
    m_uvPoll->data = this;
}
}
