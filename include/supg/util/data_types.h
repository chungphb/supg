//
// Created by chungphb on 25/5/21.
//

#pragma once

#include <string>
#include <cstring>
#include <vector>

namespace supg {

using byte = char;
using byte_array = std::basic_string<byte>;

enum struct message_type {
    join_request,
    join_accept,
    unconfirmed_data_up,
    unconfirmed_data_down,
    confirmed_data_up,
    confirmed_data_down,
    rejoin_request,
    proprietary
};

enum struct log_level {
    panic = 0,
    fatal = 1,
    error = 2,
    warning = 3,
    info = 4,
    debug = 5
};

struct network_server {
    std::string _host;
    uint16_t _port;
};

}