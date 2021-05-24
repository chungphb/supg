//
// Created by chungphb on 21/5/21.
//

#include <supg/core/gateway.h>
#include <supg/util/data_types.h>
#include <iostream>
#include <unistd.h>

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