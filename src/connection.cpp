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

    std::cout << "Connected successfully" << std::endl;
}

RPC::IPCConnection::~IPCConnection() { ::close(descriptor); }

bool RPC::IPCConnection::Write(const void *data, const size_t length) const {
    const auto sent = send(descriptor, data, length, 0);
    if (sent < 0)
        throw std::runtime_error("Failed to send data to IPC Socket");
    return sent != (ssize_t)length;
}

bool RPC::IPCConnection::Read(void *data, const size_t length) const {
    const auto success = recv(descriptor, data, length, 0);
    if (success <= 0)
        throw std::runtime_error("Failed to receive data from IPC Socket");
    return success != (int)length;
}

RPC::IPCConnection &RPC::IPCConnection::get() {
    static IPCConnection instance{};
    return instance;
}

RPC::Client::Client() : connection(RPC::IPCConnection::get()) {}

bool RPC::Client::send(const std::string_view data, const OPCode code) const {

    const auto s = static_cast<uint32_t>(data.length());
    if (s > Frame::FrameSize)
        throw std::runtime_error("Frame message exceeds length limit");

    Frame frame{{code, s}, {}};
    memcpy(frame.message, data.data(), s);

    return connection.Write(&frame, s + sizeof(FrameHeader));
}

bool RPC::Client::recv(RPC::Frame &buffer) const {
    if (state != RPC::State::Connected && state != RPC::State::SentHandshake)
        return false;

    bool did_read {false};
    for (;;) {
        did_read = connection.Read(&buffer, sizeof(FrameHeader));
        if (!did_read) {
            
        }
    }
}
