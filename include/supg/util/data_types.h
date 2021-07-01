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

struct network_server {
    std::string _host;
    uint16_t _port;
};

}