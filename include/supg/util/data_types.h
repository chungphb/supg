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

constexpr uint16_t DEFAULT_PORT = 1700;
constexpr size_t GATEWAY_MAC_LEN = 8;
constexpr size_t BUFFER_SIZE = 1024;

struct network_server {
    std::string _host;
    uint16_t _port;
};

}