//
// Created by chungphb on 25/5/21.
//

#pragma once

#include <supg/core/gateway.h>

namespace supg {

struct device {
public:
    device(std::vector<byte> addr);
    void send_payload(const gateway& gateway, int socket_fd, const sockaddr_in& server_addr, rxpk&& rxpk);
    payload generate_payload(message_type m_type, byte_array&& data);

private:
    std::vector<byte> _addr;
    uint16_t _frame_cnt = 0;
};

}
