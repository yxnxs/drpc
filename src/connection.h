#ifndef DRPC__CONNECTION__H
#define DRPC__CONNECTION__H

#include "constants.hpp"
#include <filesystem>

namespace RPC {

class IPCConnection {
    int descriptor{-1};

    IPCConnection();

  public:
    ~IPCConnection();

    IPCConnection(const IPCConnection &) = delete;
    void operator=(const IPCConnection &) = delete;

    bool Write(const void *data, const size_t length) const;
    bool Read(void *data, const size_t lenth) const;

    static IPCConnection &get();
};

struct FrameHeader {
    OPCode frame_code{};
    uint32_t frame_length{};
};

struct Frame {
    static constexpr auto FrameSize = 64 * 1024 - sizeof(FrameHeader);

    FrameHeader header{};
    char message[FrameSize];
};

class Client {
    const IPCConnection &connection;
    State state{RPC::State::Disconnected};

  public:
    Client();

    bool send(const std::string_view data,
              const OPCode code = OPCode::Handshake) const;

    bool recv(Frame &buffer) const;
};

}; // namespace RPC

#endif // DRPC__CONNECTION__H
