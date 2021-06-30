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
    // Parse config
    std::ifstream ifs("supg.toml");
    toml::ParseResult res = toml::parse(ifs);
    if (!res.valid()) {
        throw std::runtime_error("Config: Invalid config file");
    }
    const toml::Value& config = res.value;

    // Initialize network server
    const toml::Value* val = config.find("network_server.host");
    if (val && val->is<std::string>()) {
        _config._network_server._host = val->as<std::string>();
        if (_config._network_server._host.empty()) {
            throw std::runtime_error("Config: Invalid host");
        }
    }
    val = config.find("network_server.port");
    if (val && val->is<int>()) {
        _config._network_server._port = val->as<int>();
        if (_config._network_server._port <= 0) {
            throw std::runtime_error("Config: Invalid port");
        }
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

    // Initialize device configs
    val = config.find("simulator.device.first_eui");
    if (val && val->is<std::string>()) {
        _config._first_dev_eui = val->as<std::string>();
        if (_config._first_dev_eui.size() != 16) {
            throw std::runtime_error("Config: Invalid device EUI");
        }
    }
    val = config.find("simulator.device.first_addr");
    if (val && val->is<std::string>()) {
        _config._first_dev_addr = val->as<std::string>();
        if (_config._first_dev_addr.size() != 16) {
            throw std::runtime_error("Config: Invalid device address");
        }
    }
    val = config.find("simulator.device.first_network_session_key");
    if (val && val->is<std::string>()) {
        _config._first_dev_nwk_s_key = val->as<std::string>();
        if (_config._first_dev_nwk_s_key.size() != 32) {
            throw std::runtime_error("Config: Invalid device network session key");
        }
    }
    val = config.find("simulator.device.first_application_session_key");
    if (val && val->is<std::string>()) {
        _config._first_dev_app_s_key = val->as<std::string>();
        if (_config._first_dev_app_s_key.size() != 32) {
            throw std::runtime_error("Config: Invalid device application session key");
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
    val = config.find("simulator.gateway.first_id");
    if (val && val->is<std::string>()) {
        _config._first_gw_id = val->as<std::string>();
        if (_config._first_gw_id.size() != 16) {
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

    // Initialize gateway list
    auto gateway_id = from_hex_string_to<uint64_t>(_config._first_gw_id);
    for (int i = 0; i < _config._dev_count; ++i, ++gateway_id) {
        auto gw = std::make_shared<gateway>(to_hex_string(gateway_id), _config);
        _gw_list.emplace_back(gw);
    }

    // Initialize device list
    auto dev_eui = from_hex_string_to<uint64_t>(_config._first_dev_eui);
    auto dev_addr = from_hex_string_to<uint32_t>(_config._first_dev_addr);
    auto nwk_s_key_msb = from_hex_string_to<uint64_t>(_config._first_dev_nwk_s_key.substr(0, 16));
    auto nwk_s_key_lsb = from_hex_string_to<uint64_t>(_config._first_dev_nwk_s_key.substr(16, 16));
    auto app_s_key_msb = from_hex_string_to<uint64_t>(_config._first_dev_nwk_s_key.substr(0, 16));
    auto app_s_key_lsb = from_hex_string_to<uint64_t>(_config._first_dev_app_s_key.substr(16, 16));
    for (int i = 0; i < _config._dev_count; ++i, ++dev_eui, ++dev_addr, ++nwk_s_key_lsb, ++app_s_key_lsb) {
        auto dev = std::make_shared<device>(to_hex_string(dev_eui), _config);

        // Set device address
        auto buf = to_hex_string(dev_addr);
        for (size_t j = 0; j < 4; ++j) {
            dev->_addr[j] = static_cast<byte>(from_hex_string_to<uint32_t>(buf.substr(j << 1, 2)));
        }

        // Set network session key
        buf = to_hex_string(nwk_s_key_msb);
        for (size_t j = 0; j < 8; ++j) {
            dev->_nwk_s_key[j] = static_cast<byte>(from_hex_string_to<uint32_t>(buf.substr(j << 1, 2)));
        }
        buf = to_hex_string(nwk_s_key_lsb);
        for (size_t j = 8; j < 16; ++j) {
            dev->_nwk_s_key[j] = static_cast<byte>(from_hex_string_to<uint32_t>(buf.substr((j - 8) << 1, 2)));
        }

        // Set application session key
        buf = to_hex_string(app_s_key_msb);
        for (size_t j = 0; j < 8; ++j) {
            dev->_app_s_key[j] = static_cast<byte>(from_hex_string_to<uint32_t>(buf.substr(j << 1, 2)));
        }
        buf = to_hex_string(app_s_key_lsb);
        for (size_t j = 8; j < 16; ++j) {
            dev->_app_s_key[j] = static_cast<byte>(from_hex_string_to<uint32_t>(buf.substr((j - 8) << 1, 2)));
        }

        // Set gateways
        auto gw_count = get_random_number(_config._gw_min_count, _config._gw_max_count);
        std::vector<size_t> gw_list;
        for (int j = 0; j < gw_count; ++j) {
            auto rand_id = get_random_number(0, _config._gw_max_count - 1);
            auto gw_it = std::find_if(gw_list.begin(), gw_list.end(), [rand_id](auto gw_id) {
                return rand_id == gw_id;
            });
            if (gw_it == gw_list.end()) {
                dev->_gateways.push_back(_gw_list[rand_id]);
            } else {
                --j;
            }
        }

        // Add device
        _dev_list.emplace_back(dev);
    }

    // Log config
    spdlog::debug("[Config] {:<25}:{:>35}", "Network server", to_string(_config._network_server));
    spdlog::debug("[Config] {:<25}:{:>35}", "First device", _config._first_dev_eui);
    spdlog::debug("[Config] {:<25}:{:>35}", "Device count", _config._dev_count);
    spdlog::debug("[Config] {:<25}:{:>35}", "First gateway", _config._first_gw_id);
    spdlog::debug("[Config] {:<25}:{:>35}", "Gateway count", _config._gw_max_count);
}

void simulator::run() {
    if (_stopped) {
        return;
    }

    // Run gateways
    for (const auto& gw : _gw_list) {
        gw->run();
    }

    // Run devices
    for (const auto& dev : _dev_list) {
        dev->run();
    }
}

void simulator::stop() {
    if (_stopped) {
        return;
    }

    _stopped = true;

    // Stop devices
    for (const auto& dev : _dev_list) {
        dev->stop();
    }

    // Stop gateways
    for (const auto& gw : _gw_list) {
        gw->stop();
    }
}

}