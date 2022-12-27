#ifndef DRPC__CONSTANTS__HPP
#define DRPC__CONSTANTS__HPP

#include <stdint.h>

namespace RPC {

enum class OPCode : uint32_t {
    Handshake = 0,
    Frame = 1,
    Close = 2,
    Ping = 3,
    Pong = 4,
};

enum class State : uint32_t {
    Disconnected = 0,
    SentHandshake,
    AwaitingResponse,
    Connected,
};
}; // namespace RPC

#endif // DRPC__CONSTANTS__HPP
