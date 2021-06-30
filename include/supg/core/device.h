//
// Created by chungphb on 25/5/21.
//

#pragma once

#include <supg/core/gateway.h>

namespace supg {

struct device {
public:
    device(const std::string& addr, const config& config);
    void send_payload(const gateway& gateway, int socket_fd, const sockaddr_in& server_addr);

private:
    payload generate_payload(message_type m_type);

private:
    std::array<byte, 4> _addr;
    int _frame_cnt = 0;
    const config& _config;
};

}
