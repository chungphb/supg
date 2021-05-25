//
// Created by chungphb on 25/5/21.
//

#include <supg/util/functions.h>
#include <random>
#include <sstream>
#include <iomanip>

namespace supg {

size_t get_random(size_t min, size_t max) {
    std::random_device rd;
    std::mt19937 mt{rd()};
    std::uniform_int_distribution<size_t> dist{min, max};
    return dist(mt);
}

byte get_random_byte() {
    size_t res = get_random(0, 127);
    return static_cast<byte>(res);
}

std::string get_hex_string(const std::vector<byte>& vec) {
    std::stringstream ss;
    for (const auto& ele : vec) {
        ss << std::hex << std::setw(2) << std::setfill('0') << (int)ele;
    }
    return ss.str();
}

}