//
// Created by chungphb on 25/5/21.
//

#include <supg/core/device.h>

namespace supg {

device::device(std::vector<byte> addr) : _addr{std::move(addr)} {}

void device::send_payload(const gateway& gateway, int socket_fd, const sockaddr_in& server_addr, rxpk&& rxpk) {
    auto&& payload = generate_payload(message_type::unconfirmed_data_up, "data");
    rxpk.update_payload(std::move(payload));
    gateway.push_data(socket_fd, server_addr, std::move(rxpk));
}

payload device::generate_payload(message_type m_type, byte_array&& data) {
    payload new_payload;
    new_payload._f_cnt = _frame_cnt++;
    new_payload._dev_addr = _addr;
    new_payload._m_type = m_type;
    new_payload._data = std::move(data);
    return new_payload;
}

}