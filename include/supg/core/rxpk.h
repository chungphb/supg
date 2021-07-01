//
// Created by chungphb on 25/5/21.
//

#pragma once

#include <supg/core/payload.h>
#include <string>

namespace supg {

struct rxpk {
public:
    template <typename field_value_t>
    void add(std::string field_name, field_value_t field_value) {
        _fields.emplace_back(std::move(field_name), std::to_string(field_value));
    }
    void add(std::string field_name, const std::string& field_value);
    void add(std::string field_name, const char* field_value);
    std::string string() const;

private:
    std::vector<std::pair<std::string, std::string>> _fields;
};

}