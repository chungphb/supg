//
// Created by chungphb on 25/5/21.
//

#include <supg/core/gateway.h>
#include <supg/util/functions.h>
#include <iostream>
#include <iomanip>
#include <algorithm>

namespace supg {

gateway::gateway(std::vector<byte> mac) : _mac{std::move(mac)} {}

void gateway::push_data(int sock_fd, const sockaddr_in& server_addr, rxpk&& rxpk) const {
    auto packet_id = rxpk.get_payload()._f_cnt;
    auto dev_addr = get_hex_string(rxpk.get_payload()._dev_addr);

    // Create a Semtech UDP packet
    byte_array packet(GATEWAY_MAC_LEN + 4, 0);
    packet[0] = 0x02;
    packet[1] = get_random_byte();
    packet[2] = get_random_byte();
    packet[3] = 0x00;
    for (size_t i = 0; i < GATEWAY_MAC_LEN; ++i) {
        packet[i + 4] = _mac[i];
    }
    packet += rxpk.as_byte_array();

    // Send packet
    sendto(sock_fd, packet.c_str(), packet.length(), MSG_CONFIRM, (const sockaddr*)&server_addr, sizeof(server_addr));
    std::cout << "Gateway " << get_hex_string(_mac) << ": ";
    std::cout << "Send packet #" << packet_id << " from device " << dev_addr << std::endl;

    // Receive ACK
    size_t ack_len, addr_len;
    char ack[BUFFER_SIZE];
    ack_len = recvfrom(sock_fd, ack, BUFFER_SIZE, MSG_WAITALL, (sockaddr*)&server_addr, (socklen_t*)&addr_len);
    ack[ack_len] = '\0';
    std::cout << "Gateway " << get_hex_string(_mac) << ": ";
    std::cout << "Receive ACK for packet #" << packet_id << " of device " << dev_addr << std::endl;
}

}
