//
// Created by chungphb on 26/5/21.
//

#pragma once

#include <supg/util/data_types.h>

namespace supg {

constexpr size_t DEV_ADDR_LEN = 4;
constexpr size_t GW_MAC_LEN = 8;
constexpr size_t ACK_MAX_LEN = 1024;

constexpr char NS_DEFAULT_HOST[] = "localhost";
constexpr size_t NS_DEFAULT_PORT = 1700;

struct config {
    log_level _log_level;
    std::string _network_session_key;
    std::string _application_session_key;
    int _dev_count = 1000;
    int _f_port = 10;
    std::string _payload = "TEST_PACKET_1234";
    int _freq = 868100000;
    int _s_factor = 12;
    int _gw_min_count = 3;
    int _gw_max_count = 5;
};

}