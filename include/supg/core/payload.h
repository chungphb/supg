//
// Created by chungphb on 24/5/21.
//

#pragma once

#include <supg/util/data_types.h>
#include <string>
#include <vector>

namespace supg {

struct device;
struct gateway;

struct payload {
public:
    byte_array as_byte_array() const;
    friend device;
    friend gateway;

private:
    uint16_t _f_cnt;
    std::vector<byte> _dev_addr;
    message_type _m_type;
    byte_array _data;
};

}