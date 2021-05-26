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
    void start();
    void stop();

private:
    int _socket_fd;
    sockaddr_in _server_addr;
    config _config;
    std::vector<device> _dev_list;
    std::vector<gateway> _gw_list;
};

}