//
// Created by chungphb on 25/5/21.
//

#include <supg/core/device.h>
#include <supg/util/helper.h>

namespace supg {

device::device(const std::string& addr, const config& config) : _config{config} {
    for (int i = 0; i < _addr.size(); ++i) {
        _addr[i] = static_cast<byte>(from_hex_string_to<uint32_t>(addr.substr(i << 1, 2)));
    }
}

void device::send_payload(const gateway& gateway, int socket_fd, const sockaddr_in& server_addr) {
    auto&& payload = generate_payload(message_type::unconfirmed_data_up);
    gateway.push_data(socket_fd, server_addr, std::move(payload));
}

payload device::generate_payload(message_type m_type) {
    payload new_payload;
    switch (m_type) {
        case message_type::unconfirmed_data_up: {
            new_payload._f_port = _config._f_port;
            new_payload._f_cnt = _frame_cnt++;
            new_payload._dev_addr = _addr;
            new_payload._m_type = m_type;
            new_payload._data = _config._payload;
            break;
        }
        default: {
            break;
        }
    }
    return new_payload;
}

}