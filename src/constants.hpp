#ifndef DRPC__CONSTANTS__HPP
#define DRPC__CONSTANTS__HPP

#include <stdint.h>

namespace RPC {

enum class OPCode : uint32_t {
    Handshake,
    Frame,
    Close,
    Ping,
    Pong,
};

enum class State : uint32_t {
    Disconnected,
    SentHandshake,
    AwaitingResponse,
    Connected,
};
}; // namespace RPC

#endif // DRPC__CONSTANTS__HPP
