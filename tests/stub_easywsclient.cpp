#include "../library/easywsclient.hpp"

#include <vector>

namespace easywsclient {
class DummyWebSocket : public WebSocket {
public:
    void poll(int timeout) override {
        (void)timeout;
    }

    void send(const std::string &message) override {
        (void)message;
    }

    void sendBinary(const std::string &message) override {
        (void)message;
    }

    void sendBinary(const std::vector<uint8_t> &message) override {
        (void)message;
    }

    void sendPing() override {
    }

    void close() override {
    }

    readyStateValues getReadyState() const override {
        return OPEN;
    }

    void _dispatch(Callback_Imp &callable) override {
        (void)callable;
    }

    void _dispatchBinary(BytesCallback_Imp &callable) override {
        (void)callable;
    }
};

WebSocket::pointer WebSocket::create_dummy() {
    static DummyWebSocket dummy;
    return &dummy;
}

WebSocket::pointer WebSocket::from_url(const std::string &url,
                                       const std::string &origin) {
    (void)url;
    (void)origin;
    return new DummyWebSocket();
}

WebSocket::pointer WebSocket::from_url_no_mask(const std::string &url,
                                               const std::string &origin) {
    (void)url;
    (void)origin;
    return new DummyWebSocket();
}
} // namespace easywsclient
