//
// Created by chungphb on 25/5/21.
//

#pragma once

#include <supg/core/gateway.h>
#include <memory>

namespace supg {

struct simulator;

struct device {
public:
    void run();
    void stop();
    friend struct simulator;

private:
    void send_data();
    void send_uplink(lora::phy_payload&& phy_payload);

private:
    lora::eui64 _dev_eui;
    lora::dev_addr _dev_addr;
    lora::aes128key _nwk_s_key;
    lora::aes128key _app_s_key;
    std::vector<std::shared_ptr<gateway>> _gateways;
    gw::uplink_tx_info _uplink_tx_info;
    std::vector<byte> _payload;
    uint8_t _f_port = 0;
    int _f_cnt = 0;
    bool _stopped = false;
};

}
