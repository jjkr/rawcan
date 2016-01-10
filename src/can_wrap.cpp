#include "can_wrap.hpp"
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

using node::Environment;
using v8::Local;
using v8::Function;
using v8::FunctionTemplate;

namespace rawcan
{
Nan::Persistent<Function> CANWrap::s_constructor;

NAN_MODULE_INIT(CANWrap::Initialize)
{
    Local<FunctionTemplate> tpl = Nan::New<FunctionTemplate>(New);
    tpl->SetClassName(Nan::New("CANWrap").ToLocalChecked());
    tpl->InstanceTemplate()->SetInternalFieldCount(1);

    SetPrototypeMethod(tpl, "bind", Bind);
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

    auto canAddr = sockaddr_can();
    canAddr.can_family = AF_CAN;
    canAddr.can_ifindex = ifr.ifr_ifindex;

    err = bind(obj->m_socket, reinterpret_cast<struct sockaddr*>(&canAddr),
               sizeof(canAddr));
}
}
