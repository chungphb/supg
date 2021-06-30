//
// Created by chungphb on 26/5/21.
//

#pragma once

#include <supg/util/data_types.h>
#include <array>

namespace supg {


constexpr char default_ns_host[] = "localhost";
constexpr size_t default_ns_port = 1700;

struct config {
    network_server _network_server{default_ns_host, default_ns_port};
    std::string _first_dev_eui;
    std::string _first_dev_addr;
    std::string _first_dev_nwk_s_key;
    std::string _first_dev_app_s_key;
    int _dev_count = 1;
    int _f_port = 10;
    std::string _payload = "test_packet_1234";
    int _freq = 868100000;
    int _bandwidth = 125;
    int _s_factor = 12;
    std::string _first_gw_id;
    int _gw_min_count = 1;
    int _gw_max_count = 1;
};

}