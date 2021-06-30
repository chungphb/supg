//
// Created by chungphb on 25/5/21.
//

#pragma once

#include <string>
#include <cstring>
#include <climits>
#include <vector>
#include <array>

namespace supg {

using byte = char;
constexpr byte BYTE_MIN = CHAR_MIN;
constexpr byte BYTE_MAX = CHAR_MAX;
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

struct network_server {
    std::string _host;
    uint16_t _port;
};

}