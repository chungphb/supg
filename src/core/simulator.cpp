//
// Created by chungphb on 25/5/21.
//

#include <supg/core/simulator.h>
#include <supg/util/functions.h>
#include <unistd.h>
#include <iostream>

namespace supg {

simulator::simulator() {
    // Create socket file descriptor
    if ((_socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cerr << "Failed to create socket file descriptor" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Update server information
    network_server ns{"localhost", DEFAULT_PORT};
    _server_addr.sin_family = AF_INET;
    _server_addr.sin_port = htons(ns._port);
    if (strcmp(ns._host.c_str(), "localhost") == 0) {
        _server_addr.sin_addr.s_addr = INADDR_ANY;
    } else {
        inet_aton(ns._host.c_str(), &_server_addr.sin_addr);
    }

    // Initialize device list
    _dev_list.push_back({{0x00, 0x00, 0x00, 0x00}});
    _dev_list.push_back({{0x00, 0x00, 0x00, 0x01}});
    _dev_list.push_back({{0x00, 0x00, 0x00, 0x02}});
    _dev_list.push_back({{0x00, 0x00, 0x00, 0x03}});

    // Initialize gateway list
    _gw_list.push_back({{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}});
    _gw_list.push_back({{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01}});
    _gw_list.push_back({{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02}});
    _gw_list.push_back({{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03}});

    // Initialize packet list
    rxpk pk;
    pk.add("time", "2021-01-21T16:21:17.530974Z");
    pk.add("tmst", 3512348514);
    pk.add("chan", 9);
    pk.add("rfch", 1);
    pk.add("freq", 869.1);
    pk.add("stat", 1);
    pk.add("modu", "FSK");
    pk.add("datr", 50000);
    pk.add("rssi", -75);
    pk.add("size", 16);
    _pk_list.push_back(pk);

    pk = rxpk{};
    pk.add("time", "2021-01-21T16:21:17.532038Z");
    pk.add("tmst", 3316387610);
    pk.add("chan", 0);
    pk.add("rfch", 0);
    pk.add("freq", 863.00981);
    pk.add("stat", 1);
    pk.add("modu", "LORA");
    pk.add("datr", "SF10BW125");
    pk.add("codr", "4/7");
    pk.add("rssi", -75);
    pk.add("lsnr", 5.5);
    pk.add("size", 16);
    _pk_list.push_back(pk);

    pk = rxpk{};
    pk.add("time", "2021-01-21T16:21:17.538002Z");
    pk.add("tmst", 3512348611);
    pk.add("chan", 2);
    pk.add("rfch", 0);
    pk.add("freq", 866.349812);
    pk.add("stat", 1);
    pk.add("modu", "LORA");
    pk.add("datr", "SF7BW125");
    pk.add("codr", "4/6");
    pk.add("rssi", -35);
    pk.add("lsnr", 5.1);
    pk.add("size", 16);
    _pk_list.push_back(pk);
}

simulator::~simulator() {
    close(_socket_fd);
}

void simulator::run() {
    for (auto& dev : _dev_list) {
        auto gw_id = get_random(0, _gw_list.size() - 1);
        auto pk_id = get_random(0, _pk_list.size() - 1);
        dev.send_payload(_gw_list[gw_id], _socket_fd, _server_addr, _pk_list[pk_id].clone());
    }
}

}