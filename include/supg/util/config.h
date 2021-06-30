//
// Created by chungphb on 26/5/21.
//

#pragma once

#include <supg/util/data_types.h>
#include <array>

namespace supg {

constexpr size_t DEV_ADDR_LEN = 4;
constexpr size_t GW_MAC_LEN = 8;
constexpr size_t ACK_MAX_LEN = 1024;

constexpr char NS_DEFAULT_HOST[] = "localhost";
constexpr size_t NS_DEFAULT_PORT = 1700;

struct config {
    std::string _nwk_s_key;
    std::string _app_s_key;
    std::string _first_dev;
    int _dev_count = 100;
    int _f_port = 10;
    std::string _payload = "test_packet_1234";
    int _freq = 868100000;
    int _bandwidth = 125;
    int _s_factor = 12;
    std::string _first_gw;
    int _gw_min_count = 3;
    int _gw_max_count = 5;
};

}