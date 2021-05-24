//
// Created by chungphb on 25/5/21.
//

#include <supg/core/gateway.h>
#include <iostream>
#include <iomanip>
#include <random>
#include <algorithm>

namespace supg {

char get_random_byte() {
    std::random_device rd;
    std::mt19937 mt{rd()};
    std::uniform_int_distribution<char> uid{0, 127};
    return uid(mt);
}

gateway::gateway(std::vector<byte> mac) : _mac{std::move(mac)} {}

void gateway::push_data(int& sock_fd, sockaddr_in& server_addr, size_t payload_id) const {
    // Create a Semtech UDP packet
    byte_array packet(GATEWAY_MAC_LEN + 4, 0);
    packet[0] = 0x02;
    packet[1] = get_random_byte();
    packet[2] = get_random_byte();
    packet[3] = 0x00;
    for (size_t i = 0; i < GATEWAY_MAC_LEN; ++i) {
        packet[i + 4] = _mac[i];
    }
    auto payload = UPLINK_PAYLOAD_LIST[payload_id];
    payload.erase(std::remove_if(payload.begin(), payload.end(), ::isspace), payload.end());
    packet += payload;

    // Send packet
    sendto(sock_fd, packet.c_str(), packet.length(), MSG_CONFIRM, (const sockaddr*)&server_addr, sizeof(server_addr));
    std::cout << "Gateway " << *this << ": Send packet #" << payload_id << std::endl;

    // Receive ACK
    size_t ack_len, addr_len;
    char ack[BUFFER_SIZE];
    ack_len = recvfrom(sock_fd, ack, BUFFER_SIZE, MSG_WAITALL, (sockaddr*)&server_addr, (socklen_t*)&addr_len);
    ack[ack_len] = '\0';
    std::cout << "Gateway " << *this << ": Receive ACK for packet #" << payload_id << std::endl;
}

std::ostream& operator<<(std::ostream& os, const gateway& gw) {
    for (const auto& c : gw._mac) {
        os << std::hex << std::setw(2) << std::setfill('0') << (int)c;
    }
    return os;
}

}
