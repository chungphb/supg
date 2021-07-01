//
// Created by chungphb on 24/5/21.
//

#pragma once

#include <supg/core/rxpk.h>
#include <supg/core/frame.h>
#include <supg/core/payload.h>
#include <supg/util/config.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace supg {

struct simulator;

struct gateway {
public:
    void run();
    void stop();
    void send_uplink_frame(gw::uplink_frame frame);
    friend struct simulator;

private:
    std::vector<byte> generate_push_data_packet(const gw::uplink_frame& frame);
    bool is_push_ack(const byte* resp, size_t resp_len, const std::vector<byte>& packet);

private:
    lora::eui64 _gateway_id;
    int _socket_fd;
    sockaddr_in _server;
    gw::uplink_rx_info _uplink_rx_info;
    bool _stopped = false;
};

}

