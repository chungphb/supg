//
// Created by chungphb on 24/5/21.
//

#pragma once

#include <supg/core/rxpk.h>
#include <supg/core/frame.h>
#include <supg/core/payload.h>
#include <supg/core/config.h>
#include <supg/util/channel.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <future>
#include <map>

namespace supg {

struct simulator;

struct gateway {
public:
    void run();
    void stop();
    void add_device(lora::eui64 dev_eui, std::shared_ptr<channel<gw::downlink_frame>> channel);
    void send_uplink_frame(gw::uplink_frame frame);
    void keep_alive();
    void handle_downlink_frame();
    void send_downlink_tx_ack(gw::downlink_tx_ack ack);
    friend struct simulator;

private:
    std::vector<byte> generate_push_data_packet(const gw::uplink_frame& frame);
    std::vector<byte> generate_pull_data_packet();
    std::vector<byte> generate_tx_ack_packet(const gw::downlink_tx_ack& ack);
    bool is_push_ack(const byte* resp, size_t resp_len, const std::vector<byte>& packet);
    bool is_pull_ack(const byte* resp, size_t resp_len);
    bool is_pull_resp(const byte* resp, size_t resp_len);

private:
    int _push_socket_fd = -1;
    int _pull_socket_fd = -1;
    sockaddr_in _server;
    lora::eui64 _gateway_id;
    std::map<lora::eui64, std::shared_ptr<channel<gw::downlink_frame>>> _devices;
    gw::uplink_rx_info _uplink_rx_info;
    std::future<void> _keep_alive;
    std::future<void> _handle_downlink_frame;
    bool _connected = false;
    bool _stopped = false;
    std::mutex _push_mutex;
};

}

