#include "test_common.h"

TEST(WebSocketSenderBasics) {
    WebSocketSender sender;
    sender.connect("ws://127.0.0.1:1");
    EXPECT_TRUE(sender.isConnected());
    sender.emit("hello");
    WebSocketSender sender2("ws://127.0.0.1:1");
    EXPECT_TRUE(sender2.isConnected());
    sender2.send("ping");
}

TEST(WebSocketSenderAddressParsing) {
    WebSocketSender sender;
    sender.setAddressAndPort("127.0.0.1:1234");
    expect_exit([&sender] { sender.setAddressAndPort("127.0.0.1"); }, 1);
}

TEST(WebSocketSenderParsing) {
    WebSocketSender sender;
    sender.setAddressAndPort("127.0.0.1:8080");
    sender.setPort(1234);
    sender.setAddress("ws://example");
    expect_exit([&]() { sender.setAddressAndPort("bad"); }, 1);
}

TEST(EasyWsClientStub) {
    auto *ws = easywsclient::WebSocket::from_url("ws://example", "");
    EXPECT_TRUE(ws != nullptr);
    ws->poll(1);
    ws->send("hi");
    ws->sendBinary(std::string("bin"));
    std::vector<uint8_t> bytes = {1, 2, 3};
    ws->sendBinary(bytes);
    ws->sendPing();
    EXPECT_EQ(ws->getReadyState(), easywsclient::WebSocket::OPEN);
    ws->close();
    delete ws;

    auto *ws2 = easywsclient::WebSocket::from_url_no_mask("ws://example", "");
    EXPECT_TRUE(ws2 != nullptr);
    ws2->close();
    delete ws2;

    auto *dummy = easywsclient::WebSocket::create_dummy();
    dummy->poll(0);
    dummy->close();
}
