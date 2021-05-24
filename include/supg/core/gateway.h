//
// Created by chungphb on 24/5/21.
//

#pragma once

#include <supg/util/payload.h>
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
    void push_data(int& sock_fd, sockaddr_in& server_addr, size_t payload_id) const;
    friend std::ostream& operator<<(std::ostream& os, const gateway& gw);

private:
    std::vector<byte> _mac;
};

}

