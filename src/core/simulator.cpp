//
// Created by chungphb on 25/5/21.
//

#include <supg/core/simulator.h>
#include <supg/util/helper.h>
#include <toml/toml.h>
#include <spdlog/spdlog.h>
#include <unistd.h>

namespace supg {

void simulator::init() {
    // Create socket file descriptor
    if ((_socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        spdlog::critical("Failed to create socket file descriptor");
        exit(EXIT_FAILURE);
    }

    // Parse config
    std::ifstream ifs("supg.toml");
    toml::ParseResult res = toml::parse(ifs);
    if (!res.valid()) {
        throw std::runtime_error("Config: Invalid config file");
    }
    const toml::Value& config = res.value;

    // Initialize network server
    network_server ns{NS_DEFAULT_HOST, NS_DEFAULT_PORT};
    const toml::Value* val = config.find("network_server.host");
    if (val && val->is<std::string>()) {
        ns._host = val->as<std::string>();
        if (ns._host.empty()) {
            throw std::runtime_error("Config: Invalid host");
        }
    }
    val = config.find("network_server.port");
    if (val && val->is<int>()) {
        ns._port = val->as<int>();
        if (ns._port <= 0) {
            throw std::runtime_error("Config: Invalid port");
        }
    }
    _server_addr.sin_family = AF_INET;
    _server_addr.sin_port = htons(ns._port);
    if (strcmp(ns._host.c_str(), NS_DEFAULT_HOST) == 0) {
        _server_addr.sin_addr.s_addr = INADDR_ANY;
    } else {
        inet_aton(ns._host.c_str(), &_server_addr.sin_addr);
    }

    // Initialize log level
    spdlog::set_level(spdlog::level::info);
    val = config.find("general.log_level");
    if (val && val->is<int>()) {
        auto log_level = val->as<int>();
        if (log_level == 0) {
            spdlog::set_level(spdlog::level::off);
        } else if (log_level == 1) {
            spdlog::set_level(spdlog::level::critical);
        } else if (log_level == 2) {
            spdlog::set_level(spdlog::level::err);
        } else if (log_level == 3) {
            spdlog::set_level(spdlog::level::warn);
        } else if (log_level == 4) {
            spdlog::set_level(spdlog::level::info);
        } else if (log_level == 5) {
            spdlog::set_level(spdlog::level::debug);
        } else if (log_level == 6) {
            spdlog::set_level(spdlog::level::trace);
        } else {
            throw std::runtime_error("Config: Invalid log level");
        }
    }

    // Initialize NwkSKey and AppSKey
    val = config.find("simulator.network_session_key");
    if (val && val->is<std::string>()) {
        _config._nwk_s_key = val->as<std::string>();
        if (_config._nwk_s_key.size() != 32) {
            throw std::runtime_error("Config: Invalid session key");
        }
    }
    val = config.find("simulator.application_session_key");
    if (val && val->is<std::string>()) {
        _config._app_s_key = val->as<std::string>();
        if (_config._app_s_key.size() != 32) {
            throw std::runtime_error("Config: Invalid session key");
        }
    }

    // Initialize device configs
    val = config.find("simulator.device.first");
    if (val && val->is<std::string>()) {
        _config._first_dev = val->as<std::string>();
        if (_config._first_dev.size() != 8) {
            throw std::runtime_error("Config: Invalid device address");
        }
    }
    val = config.find("simulator.device.count");
    if (val && val->is<int>()) {
        _config._dev_count = val->as<int>();
        if (_config._dev_count <= 0) {
            throw std::runtime_error("Config: Invalid device count");
        }
    }
    val = config.find("simulator.device.f_port");
    if (val && val->is<int>()) {
        _config._f_port = val->as<int>();
        if (_config._f_port <= 0) {
            throw std::runtime_error("Config: Invalid frame port");
        }
    }
    val = config.find("simulator.device.payload");
    if (val && val->is<std::string>()) {
        _config._payload = val->as<std::string>();
        if (_config._payload.empty()) {
            throw std::runtime_error("Config: Invalid payload");
        }
    }
    val = config.find("simulator.device.frequency");
    if (val && val->is<int>()) {
        _config._freq = val->as<int>();
        if (_config._freq <= 0) {
            throw std::runtime_error("Config: Invalid frequency");
        }
    }
    val = config.find("simulator.device.bandwidth");
    if (val && val->is<int>()) {
        _config._bandwidth = val->as<int>();
        if (_config._bandwidth <= 0) {
            throw std::runtime_error("Config: Invalid bandwidth");
        }
    }
    val = config.find("simulator.device.spreading_factor");
    if (val && val->is<int>()) {
        _config._s_factor = val->as<int>();
        if (_config._s_factor <= 0) {
            throw std::runtime_error("Config: Invalid spreading-factor");
        }
    }

    // Initialize gateway configs
    val = config.find("simulator.gateway.first");
    if (val && val->is<std::string>()) {
        _config._first_gw = val->as<std::string>();
        if (_config._first_gw.size() != 16) {
            throw std::runtime_error("Config: Invalid gateway ID");
        }
    }
    val = config.find("simulator.gateway.min_count");
    if (val && val->is<int>()) {
        _config._gw_min_count = val->as<int>();
        if (_config._gw_min_count <= 0) {
            throw std::runtime_error("Config: Invalid gateway min count");
        }
    }
    val = config.find("simulator.gateway.max_count");
    if (val && val->is<int>()) {
        _config._gw_max_count = val->as<int>();
        if (_config._gw_max_count < _config._gw_min_count) {
            throw std::runtime_error("Config: Invalid gateway max count");
        }
    }

    std::stringstream ss;

    // Initialize device list
    auto dev_addr = from_hex_string_to<uint32_t>(_config._first_dev);
    for (int i = 0; i < _config._dev_count; ++i, ++dev_addr) {
        _dev_list.emplace_back(to_hex_string(dev_addr), _config);
    }

    // Initialize gateway list
    auto gateway_id = from_hex_string_to<uint64_t>(_config._first_gw);
    for (int i = 0; i < _config._dev_count; ++i, ++gateway_id) {
        _gw_list.emplace_back(to_hex_string(gateway_id), _config);
    }

    // Log config
    spdlog::debug("[Config] {:<25}:{:>35}", "Network server", ns._host + ":" + std::to_string(ns._port));
    spdlog::debug("[Config] {:<25}:{:>35}", "Network session key", _config._nwk_s_key);
    spdlog::debug("[Config] {:<25}:{:>35}", "Application session key", _config._app_s_key);
    spdlog::debug("[Config] {:<25}:{:>35}", "Starting device", _config._first_dev);
    spdlog::debug("[Config] {:<25}:{:>35}", "Device count", _config._dev_count);
    spdlog::debug("[Config] {:<25}:{:>35}", "Starting gateway", _config._first_gw);
    spdlog::debug("[Config] {:<25}:{:>35}", "Gateway count", _config._gw_max_count);
}

void simulator::run() {
    for (auto& dev : _dev_list) {
        auto gw_id = get_random_number<size_t>(0, _gw_list.size() - 1);
        dev.send_payload(_gw_list[gw_id], _socket_fd, _server_addr);
    }
}

void simulator::stop() {
    close(_socket_fd);
}

}