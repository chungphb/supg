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
    device(const std::string& eui, const config& config);
    void run();
    void stop();
    friend struct simulator;

private:
    payload generate_payload(message_type m_type);

private:
    std::array<byte, 8> _eui;
    std::array<byte, 4> _addr;
    std::array<byte, 16> _nwk_s_key;
    std::array<byte, 16> _app_s_key;
    const config& _config;
    std::vector<std::shared_ptr<gateway>> _gateways;
    int _frame_cnt = 0;
    bool _stopped = false;
};

}
