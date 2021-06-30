//
// Created by chungphb on 24/5/21.
//

#pragma once

#include <supg/core/rxpk.h>
#include <supg/core/payload.h>
#include <supg/util/config.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace supg {

struct gateway {
public:
    gateway(const std::string& id, const config& config);
    void run();
    void stop();
    void push_data(const payload& payload) const;

private:
    rxpk generate_data(byte_array&& payload) const;

private:
    std::array<byte, 8> _id;
    const config& _config;
    int _socket_fd;
    sockaddr_in _server;
    bool _stopped = false;
};

}

