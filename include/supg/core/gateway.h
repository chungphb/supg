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
    gateway(const std::string& mac, const config& config);
    void push_data(int sock_fd, const sockaddr_in& server_addr, payload&& payload) const;

private:
    rxpk generate_data(byte_array&& payload) const;

private:
    std::array<byte, 8> _mac;
    const config& _config;
};

}

