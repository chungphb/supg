//
// Created by chungphb on 25/5/21.
//

#include <supg/core/gateway.h>
#include <supg/util/helper.h>
#include <spdlog/spdlog.h>
#include <algorithm>

namespace supg {

gateway::gateway(const std::string& id, const config& config) : _config{config} {
    for (int i = 0; i < 8; ++i) {
        _id[i] = static_cast<byte>(from_hex_string_to<uint32_t>(id.substr(i << 1, 2)));
    }
}

void gateway::run() {
    // Create socket file descriptor
    if ((_socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        spdlog::critical("Failed to create socket file descriptor");
        exit(EXIT_FAILURE);
    }

    // Initialize server address
    _server.sin_family = AF_INET;
    _server.sin_port = htons(_config._network_server._port);
    if (strcmp(_config._network_server._host.c_str(), default_ns_host) == 0) {
        _server.sin_addr.s_addr = INADDR_ANY;
    } else {
        inet_aton(_config._network_server._host.c_str(), &_server.sin_addr);
    }
}

void gateway::stop() {
    _stopped = true;
    close(_socket_fd);
}

void gateway::push_data(const payload& payload) const {
    auto pk_id = payload._f_cnt;
    auto&& gw_id = to_hex_string(_id.data(), _id.size());
    auto&& dev_addr = to_hex_string(payload._dev_addr.data(), payload._dev_addr.size());

    // Create Semtech UDP packet
    auto&& data = generate_data(payload.as_byte_array());
    byte_array packet(8 + 4, 0);
    packet[0] = 0x02;
    packet[1] = get_random_byte();
    packet[2] = get_random_byte();
    packet[3] = 0x00;
    for (size_t i = 0; i < 8; ++i) {
        packet[i + 4] = _id[i];
    }
    packet += data.as_byte_array();

    // Send packet
    sendto(_socket_fd, packet.c_str(), packet.length(), MSG_CONFIRM, (const sockaddr*)&_server, sizeof(_server));
    spdlog::info("Gateway {}: Send packet #{} from device {}", gw_id, pk_id, dev_addr);

    // Receive ACK
    byte ack[1024];
    auto ack_len = timeout_recvfrom(_socket_fd, ack, 1024, _server, 10);
    if (ack_len < 0) {
        spdlog::info("Gateway {}: Failed to receive ACK for packet #{} from device {}", gw_id, pk_id, dev_addr);
    } else {
        ack[ack_len] = '\0';
        if (ack_len == 4) {
            spdlog::info("Gateway {}: Receive valid ACK for packet #{} from device {}", gw_id, pk_id, dev_addr);
        } else {
            spdlog::info("Gateway {}: Receive invalid ACK for packet #{} from device {}", gw_id, pk_id, dev_addr);
        }
    }
}

rxpk gateway::generate_data(byte_array&& payload) const {
    rxpk data;
    data.add("time", get_current_timestamp());
    data.add("tmst", get_time_since_epoch());
    data.add("chan", 0);
    data.add("rfch", 0);
    data.add("freq", _config._freq / 1000000.);
    data.add("stat", 1);
    data.add("modu", "LORA");
    std::basic_stringstream<byte> datr;
    datr << "SF" << _config._s_factor;
    datr << "BW" << _config._bandwidth;
    data.add("datr", datr.str());
    data.add("codr", "3/4");
    data.add("rssi", 50);
    data.add("size", payload.size());
    data.add("data", base64_encode(payload));
    return data;
}

}
