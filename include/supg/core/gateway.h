//
// Created by chungphb on 24/5/21.
//

#pragma once

#include <supg/core/rxpk.h>
#include <supg/core/payload.h>
#include <supg/util/data_types.h>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace supg {

char get_random_byte();

struct gateway {
public:
    gateway(std::vector<byte> mac);
    void push_data(int sock_fd, const sockaddr_in& server_addr, rxpk&& rxpk) const;

private:
    std::vector<byte> _mac;
};

}

