#include "connection.h"
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

RPC::IPCConnection::IPCConnection() {

    descriptor = socket(AF_UNIX, SOCK_STREAM, 0);

    sockaddr_un addr{AF_UNIX, ""};

    const static auto path = []() -> std::string {
        const char *path = getenv("XDG_RUNTIME_DIR");

        path = path ? path : getenv("TMPDIR");
        path = path ? path : getenv("TMP");
        path = path ? path : getenv("TEMP");
        path = path ? path : "/tmp";

        const auto base = std::string(path).append("/discord-ipc-");

        std::string string_buffer{};
        struct stat stat_buffer {};

        for (char x = '0'; x < '9' + 1; x++) {
            string_buffer = base;
            string_buffer.append(1, x);

            if (stat(string_buffer.c_str(), &stat_buffer) == 0)
                return string_buffer;
        }

        throw std::runtime_error("No discord socket found");
    }();

    strcpy(addr.sun_path, path.c_str());

    if (connect(descriptor, (sockaddr *)&addr, sizeof(addr)) < 0)
        throw std::runtime_error("Failed to connect to IPC Socket");
}

RPC::IPCConnection::~IPCConnection() { ::close(descriptor); }

bool RPC::IPCConnection::Write(const void *data, const size_t length) const {
    const auto sent = send(descriptor, data, length, 0);
    if (sent < 0)
        throw std::runtime_error("Failed to send data to IPC Socket");
    return sent == static_cast<ssize_t>(length);
}

bool RPC::IPCConnection::Read(void *data, const size_t length) const {
    const auto received = recv(descriptor, data, length, 0);
    if (received <= 0) {
        if (errno == EAGAIN)
            return false;
        throw std::runtime_error("Failed to receive data from IPC Socket");
    }
    return received == static_cast<int>(length);
}

const RPC::IPCConnection &RPC::IPCConnection::get() {
    const static IPCConnection instance{};
    return instance;
}

RPC::Client::Client(std::string client_id)
    : connection(RPC::IPCConnection::get()), client_id(std::move(client_id)) {
    // TODO AUTHENTICATE and AUTHORIZE here
}

bool RPC::Client::send(const std::string_view data, const OPCode code) const {

    const auto s = static_cast<uint32_t>(data.length());
    if (s > Frame::FrameSize)
        throw std::runtime_error("Frame message exceeds length limit");

    Frame frame{{code, s}, {}};
    memcpy(frame.message, data.data(), s);

    return connection.Write(&frame, s + sizeof(FrameHeader));
}

bool RPC::Client::recv(RPC::Response &response_buffer) const {
    if (state != RPC::State::Connected && state != RPC::State::SentHandshake)
        return false;

    Frame buffer{};
    bool did_read{false};

    for (;;) {
        did_read = connection.Read(&buffer, sizeof(FrameHeader));
        if (!did_read)
            return false;

        const auto length = buffer.header.frame_length;

        if (length > 0) {
            did_read = connection.Read(buffer.message, length);
            if (!did_read) {
                throw std::runtime_error(
                    "Failed to read message into frame buffer");
            }
            buffer.message[length] = 0;
        }

        switch (buffer.header.frame_code) {
        case RPC::OPCode::Close:
            throw std::runtime_error("Connection closing");

        case RPC::OPCode::Frame:
            // TODO Parse frame into object here
            return true;
        case RPC::OPCode::Ping:
            buffer.header.frame_code = RPC::OPCode::Pong;
            if (!connection.Write(&buffer, buffer.header.frame_length +
                                               sizeof(RPC::FrameHeader))) {
                throw std::runtime_error("Failed to PONG");
            }
            break;

        case RPC::OPCode::Pong:
            break;
        case RPC::OPCode::Handshake:
        default:
            throw std::runtime_error("Bad IPC frame");
        }
    }
}
