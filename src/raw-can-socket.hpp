#pragma once
#include <nan.h>
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
    /**
     * Sets the receive filter for packets based on can_id
     * @param filter_id {Integer} the id to filter based on
     * @param mask {Integer} the filter mask
     */
    static NAN_METHOD(setFilter);
    /**
     * Set the receive callback
     * @param callback {any} handles a frame object e.g. { id: 0x13, data:
     * Uint8Array.from([0x3a, 0x12]) }
     */
    static NAN_METHOD(onFrame);
    /**
     * Send a can frame
     * @param frame {any} sends a frame object e.g. { id: 0x13, data: [0x3a,
     * 0x12] }
     * @param callback {any}
     */
    static NAN_METHOD(write);
    static NAN_METHOD(stop);

    static Nan::Persistent<v8::Function> s_constructor;

    const int m_socket;
    std::unique_ptr<uv_poll_t> m_uvPoll;
    int m_error = 0;
    int m_pollEvents = 0;
};
}
