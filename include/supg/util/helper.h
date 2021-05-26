//
// Created by chungphb on 25/5/21.
//

#pragma once

#include <supg/util/data_types.h>

namespace supg {

size_t get_random(size_t min, size_t max);

byte get_random_byte();

std::string get_hex_string(const std::vector<byte>& vec);

std::string get_current_timestamp();

size_t get_time();

std::string base64_encode(const std::string& in);

std::string base64_decode(const std::string& in);

}