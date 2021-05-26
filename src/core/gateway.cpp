//
// Created by chungphb on 25/5/21.
//

#include <supg/core/gateway.h>
#include <supg/util/helper.h>
#include <iostream>
#include <algorithm>

namespace supg {

gateway::gateway(std::vector<byte> mac, const config& config) : _mac{std::move(mac)}, _config{config} {}

void gateway::push_data(int sock_fd, const sockaddr_in& server_addr, payload&& payload) const {
    // Create Semtech UDP packet
    auto&& data = generate_data(payload.as_byte_array());
    byte_array packet(GW_MAC_LEN + 4, 0);
    packet[0] = 0x02;
    packet[1] = get_random_byte();
    packet[2] = get_random_byte();
    packet[3] = 0x00;
    for (size_t i = 0; i < GW_MAC_LEN; ++i) {
        packet[i + 4] = _mac[i];
    }
    packet += data.as_byte_array();

    // Send packet
    sendto(sock_fd, packet.c_str(), packet.length(), MSG_CONFIRM, (const sockaddr*)&server_addr, sizeof(server_addr));
    std::cout << "Gateway " << get_hex_string(_mac) << ": ";
    std::cout << "Send packet #" << payload._f_cnt << " from device " << get_hex_string(payload._dev_addr) << std::endl;

    // Receive ACK
    size_t ack_len, addr_len;
    char ack[ACK_MAX_LEN];
    ack_len = recvfrom(sock_fd, ack, ACK_MAX_LEN, MSG_WAITALL, (sockaddr*)&server_addr, (socklen_t*)&addr_len);
    ack[ack_len] = '\0';
    std::cout << "Gateway " << get_hex_string(_mac) << ": ";
    std::cout << "Receive ACK for packet #" << payload._f_cnt << " of device " << get_hex_string(payload._dev_addr) << std::endl;
}

rxpk gateway::generate_data(byte_array&& payload) const {
    rxpk data;
    data.add("time", get_current_timestamp());
    data.add("tmst", 3512348514);
    data.add("chan", 2);
    data.add("rfch", 0);
    data.add("freq", _config._freq / 1000000.);
    data.add("stat", 1);
    data.add("modu", "FSK");
    data.add("datr", 50000);
    data.add("rssi", -75);
    data.add("size", payload.size());
    data.add("data", base64_encode(payload));
    return data;
}

}
