//
// Created by chungphb on 25/5/21.
//

#pragma once

#include <supg/core/device.h>
#include <supg/core/gateway.h>
#include <supg/util/config.h>

namespace supg {

struct simulator {
public:
    void init();
    void run();
    void stop();

private:
    config _config;
    std::vector<std::shared_ptr<device>> _dev_list;
    std::vector<std::shared_ptr<gateway>> _gw_list;
    bool _stopped = false;
};

}