//
// Created by chungphb on 21/5/21.
//

#include "payload.h"
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <random>
#include <cstring>
#include <algorithm>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace supg {

using byte = char;
using byte_array = std::basic_string<byte>;

constexpr uint16_t DEFAULT_PORT = 1700;
constexpr size_t GATEWAY_MAC_LEN = 8;
constexpr size_t BUFFER_SIZE = 1024;

struct gateway {
public:
    gateway(std::vector<byte> mac) : _mac{std::move(mac)} {}

    void push_data(int& sock_fd, sockaddr_in& server_addr, size_t payload_id) const {
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

private:
    static char get_random_byte() {
        std::random_device rd;
        std::mt19937 mt{rd()};
        std::uniform_int_distribution<char> uid{0, 127};
        return uid(mt);
    }

    friend std::ostream& operator<<(std::ostream& os, const gateway& gw) {
        for (const auto& c : gw._mac) {
            os << std::hex << std::setw(2) << std::setfill('0') << (int)c;
        }
        return os;
    }

private:
    std::vector<byte> _mac;
};

struct network_server {
    std::string _host;
    uint16_t _port;
};

}

int main() {
    using namespace supg;

    // Create socket file descriptor
    int sock_fd;
    if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cerr << "Failed to create socket file descriptor" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Update server information
    network_server ns{"localhost", DEFAULT_PORT};
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(ns._port);
    if (strcmp(ns._host.c_str(), "localhost") == 0) {
        server_addr.sin_addr.s_addr = INADDR_ANY;
    } else {
        inet_aton(ns._host.c_str(), &server_addr.sin_addr);
    }

    // Push data
    std::vector<gateway> gws;
    gws.push_back({{0x02, 0x05, 0x01, 0x00, 0x01, 0x09, 0x09, 0x06}});
    gws.push_back({{0x00, 0x07, 0x00, 0x01, 0x01, 0x09, 0x09, 0x02}});
    for (const auto& gw : gws) {
        gw.push_data(sock_fd, server_addr, 0);
        gw.push_data(sock_fd, server_addr, 1);
        gw.push_data(sock_fd, server_addr, 2);
    }

    // Close socket file descriptor
    close(sock_fd);
    return 0;
}