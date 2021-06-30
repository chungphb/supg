//
// Created by chungphb on 25/5/21.
//

#pragma once

#include <supg/util/data_types.h>
#include <netinet/in.h>
#include <type_traits>
#include <sstream>
#include <random>
#include <iomanip>

namespace supg {

// Get random values
template <typename T = size_t>
T get_random_number(T min, T max) {
    std::random_device rd;
    std::mt19937 mt{rd()};
    std::uniform_int_distribution<T> dist{min, max};
    return dist(mt);
}

byte get_random_byte(byte min = BYTE_MIN, byte max = BYTE_MAX);

// Get time related values
std::string get_current_timestamp();
size_t get_time_since_epoch();

// Encode to and decode from base64 format
std::string base64_encode(const std::string& in);
std::string base64_decode(const std::string& in);

// Converters
template <typename T>
T from_hex_string_to(const std::string& str) {
    static_assert(std::is_integral<T>::value, "Integral required.");
    T res;
    std::stringstream ss;
    ss << std::hex << str;
    ss >> res;
    return res;
}

template <typename T>
std::string to_hex_string(T val) {
    static_assert(std::is_integral<T>::value, "Integral required.");
    std::stringstream ss;
    ss << std::setfill('0') << std::setw(sizeof(T) * 2) << std::hex << val;
    return ss.str();
}

std::string to_hex_string(const byte* str, size_t len);

// Receive UDP packet with timeout
ssize_t timeout_recvfrom(int fd, char* buf, ssize_t buf_len, const sockaddr_in& addr, int t_sec);

}