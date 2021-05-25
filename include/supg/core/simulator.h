//
// Created by chungphb on 25/5/21.
//

#pragma once

#include <supg/core/device.h>
#include <supg/core/gateway.h>

namespace supg {

struct simulator {
public:
    simulator();
    ~simulator();
    void run();

private:
    int _socket_fd;
    sockaddr_in _server_addr;
    std::vector<device> _dev_list;
    std::vector<gateway> _gw_list;
    std::vector<rxpk> _pk_list;
};

}