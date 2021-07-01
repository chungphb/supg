//
// Created by chungphb on 25/5/21.
//

#include <supg/core/rxpk.h>
#include <sstream>

namespace supg {

byte_array rxpk::as_byte_array() const {
    std::basic_stringstream<byte> ss;
    ss << R"({"rxpk":[{)";
    for (auto it = _fields.begin(); it != _fields.end();) {
        ss << R"(")" << it->first << R"(":)" << it->second;
        ss << (std::next(it) != _fields.end() ? "," : "");
        it = std::next(it);
    }
    ss << R"(}]})";
    return ss.str();
}

void rxpk::add(std::string field_name, const std::string& field_value) {
    std::basic_stringstream<byte> ss;
    ss << R"(")" << field_value << R"(")";
    _fields.emplace_back(std::move(field_name), ss.str());
}

void rxpk::add(std::string field_name, const char* field_value) {
    std::basic_stringstream<byte> ss;
    ss << R"(")" << field_value << R"(")";
    _fields.emplace_back(std::move(field_name), ss.str());
}

rxpk rxpk::clone() {
    return *this;
}

}