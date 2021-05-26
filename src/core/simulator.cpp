//
// Created by chungphb on 25/5/21.
//

#include <supg/core/simulator.h>
#include <supg/util/helper.h>
#include <toml/toml.h>
#include <unistd.h>
#include <iostream>

namespace supg {

void simulator::init() {
    // Create socket file descriptor
    if ((_socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        std::cerr << "Failed to create socket file descriptor" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Read config file
    std::ifstream config_file("supg.toml");
    toml::ParseResult res = toml::parse(config_file);
    if (!res.valid()) {
        throw std::runtime_error("Invalid config file");
    }
    const toml::Value& config = res.value;

    // Initialize network server
    network_server ns{NS_DEFAULT_HOST, NS_DEFAULT_PORT};
    const toml::Value* val = config.find("network_server.host");
    if (val && val->is<std::string>()) {
        ns._host = val->as<std::string>();
    }
    val = config.find("network_server.port");
    if (val && val->is<int>()) {
        ns._port = val->as<int>();
    }
    _server_addr.sin_family = AF_INET;
    _server_addr.sin_port = htons(ns._port);
    if (strcmp(ns._host.c_str(), NS_DEFAULT_HOST) == 0) {
        _server_addr.sin_addr.s_addr = INADDR_ANY;
    } else {
        inet_aton(ns._host.c_str(), &_server_addr.sin_addr);
    }

    // Initialize log level
    _config._log_level = log_level::info;
    val = config.find("general.log_level");
    if (val && val->is<int>()) {
        auto log_level = val->as<int>();
        if (log_level >= static_cast<int>(log_level::panic) && log_level <= static_cast<int>(log_level::debug)) {
            _config._log_level = static_cast<enum log_level>(log_level);
        }
    }

    // Initialize NwkSKey and AppSKey
    val = config.find("simulator.network_session_key");
    if (val && val->is<std::string>()) {
        _config._network_session_key = val->as<std::string>();
    }
    val = config.find("simulator.application_session_key");
    if (val && val->is<std::string>()) {
        _config._application_session_key = val->as<std::string>();
    }

    // Initialize device configs
    val = config.find("simulator.device.count");
    if (val && val->is<int>()) {
        _config._dev_count = val->as<int>();
    }
    val = config.find("simulator.device.f_port");
    if (val && val->is<int>()) {
        _config._f_port = val->as<int>();
    }
    val = config.find("simulator.device.payload");
    if (val && val->is<std::string>()) {
        _config._payload = val->as<std::string>();
    }
    val = config.find("simulator.device.frequency");
    if (val && val->is<int>()) {
        _config._freq = val->as<int>();
    }
    val = config.find("simulator.device.spreading_factor");
    if (val && val->is<int>()) {
        _config._s_factor = val->as<int>();
    }

    // Initialize gateway configs
    val = config.find("simulator.gateway.min_count");
    if (val && val->is<int>()) {
        _config._gw_min_count = val->as<int>();
    }
    val = config.find("simulator.gateway.max_count");
    if (val && val->is<int>()) {
        _config._gw_max_count = val->as<int>();
    }

    // Initialize device list
    for (int i = 0; i < _config._dev_count; ++i) {
        std::vector<byte> dev_addr;
        dev_addr.reserve(DEV_ADDR_LEN);
        for (int j = 0; j < DEV_ADDR_LEN; j++) {
            dev_addr.push_back(get_random_byte());
        }
        _dev_list.emplace_back(std::move(dev_addr), _config);
    }

    // Initialize gateway list
    for (int i = 0; i < _config._gw_max_count; ++i) {
        std::vector<byte> gw_mac;
        gw_mac.reserve(GW_MAC_LEN);
        for (int j = 0; j < GW_MAC_LEN; j++) {
            gw_mac.push_back(get_random_byte());
        }
        _gw_list.emplace_back(std::move(gw_mac), _config);
    }
}

void simulator::start() {
    for (auto& dev : _dev_list) {
        auto gw_id = get_random(0, _gw_list.size() - 1);
        dev.send_payload(_gw_list[gw_id], _socket_fd, _server_addr);
    }
}

void simulator::stop() {
    close(_socket_fd);
}

}