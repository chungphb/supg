//
// Created by chungphb on 25/5/21.
//

#include <supg/util/helper.h>
#include <date/date.h>
#include <random>
#include <sstream>
#include <iomanip>
#include <chrono>

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

std::string get_current_timestamp() {
    auto now = std::chrono::system_clock::now();
    return date::format("%FT%TZ", date::floor<std::chrono::microseconds>(now));
}

size_t get_time() {
    auto now = std::chrono::system_clock::now();
    return now.time_since_epoch().count();
}

const char* base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::string base64_encode(const std::string& in) {
    std::string out;
    unsigned int val = 0;
    int valb = -6;
    for (unsigned char c : in) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) {
        out.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    }
    while (out.size() % 4) {
        out.push_back('=');
    }
    return out;
}

std::string base64_decode(const std::string& in) {
    std::string out;
    std::vector<int> tmp(256, -1);
    for (int i = 0; i < 64; ++i) {
        tmp[base64_chars[i]] = i;
    }
    unsigned int val = 0;
    int valb = -8;
    for (unsigned char c : in) {
        if (tmp[c] == -1) {
            break;
        }
        val = (val << 6) + tmp[c];
        valb += 6;
        if (valb >= 0) {
            out.push_back(char((val >> valb) & 0xFF));
            valb -= 8;
        }
    }
    return out;
}

}