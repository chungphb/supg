//
// Created by chungphb on 21/5/21.
//

#include <iostream>
#include <string>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

constexpr uint16_t PORT = 1700;
constexpr size_t BUFFER_SIZE = 1024;

int main() {
    // Create socket file descriptor
    int sock_fd;
    if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cerr << "Failed to create socket file descriptor" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Update server information
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Send packet
    std::string packet = "Packet";
    sendto(sock_fd, packet.c_str(), packet.length(), MSG_CONFIRM, (const sockaddr*)&server_addr, sizeof(server_addr));
    std::cout << "Send packet: " << packet << std::endl;

    // Receive ACK
    size_t len, addr_len;
    char ack[BUFFER_SIZE];
    len = recvfrom(sock_fd, ack, BUFFER_SIZE, MSG_WAITALL, (sockaddr*)&server_addr, (socklen_t*)&addr_len);
    ack[len] = '\0';
    std::cout << "Receive response: " << ack << std::endl;

    close(sock_fd);
    return 0;
}