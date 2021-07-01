//
// Created by chungphb on 25/5/21.
//

#include <supg/core/gateway.h>
#include <supg/util/helper.h>
#include <spdlog/spdlog.h>
#include <algorithm>

namespace supg {

void gateway::run() {
    if ((_socket_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        spdlog::critical("Failed to create socket file descriptor");
        exit(EXIT_FAILURE);
    }
}

void gateway::stop() {
    _stopped = true;
    close(_socket_fd);
}

void gateway::send_uplink_frame(gw::uplink_frame frame) {
    // Generate PUSH_DATA packet
    frame._rx_info = _uplink_rx_info;
    auto packet = generate_push_data_packet(frame);

    // Send PUSH_DATA packet
    sendto(_socket_fd, packet.data(), packet.size(), MSG_CONFIRM, (const sockaddr*)&_server, sizeof(_server));
    spdlog::trace("GTW {}: Send PUSH_DATA packet", _gateway_id.string());

    // Handle PUSH_ACK packet
    byte resp[1024];
    auto resp_len = timeout_recvfrom(_socket_fd, resp, 1024, _server, 4);
    if (_stopped) {
        return;
    }
    if (resp_len > 0) {
        resp[resp_len] = '\0';
        if (is_push_ack(resp, resp_len, packet)) {
            spdlog::trace("GTW {}: Receive PUSH_ACK packet", _gateway_id.string());
        } else {
            spdlog::error("GTW {}: Receive invalid packet", _gateway_id.string());
        }
    } else {
        spdlog::error("GTW {}: Not receive any packet", _gateway_id.string());
    }
}

std::vector<byte> gateway::generate_push_data_packet(const gw::uplink_frame& frame) {
    std::vector<byte> packet;

    // Set protocol version
    packet.push_back(0x02);

    // Set random token
    packet.push_back(get_random_byte());
    packet.push_back(get_random_byte());

    // Set PUSH_DATA identifier
    packet.push_back(0x00);

    // Set gateway identifier
    for (const auto& byte : _gateway_id._value) {
        packet.push_back(byte);
    }

    // Set JSON object
    rxpk data;
    data.add("time", get_current_timestamp());
    data.add("tmst", get_time_since_epoch());
    data.add("chan", frame._rx_info._channel);
    data.add("rfch", frame._rx_info._rf_chain);
    data.add("freq", frame._tx_info._frequency / 1000000.);
    data.add("stat", frame._rx_info._crc_status == gw::crc_status::crc_ok ? 1 : 0);
    if (frame._tx_info._modulation == gw::modulation::lora) {
        data.add("modu", "LORA");
        std::basic_stringstream<byte> datr;
        datr << "SF" << frame._tx_info._lora_modulation_info._spreading_factor;
        datr << "BW" << frame._tx_info._lora_modulation_info._bandwidth;
        data.add("datr", datr.str());
        data.add("codr", frame._tx_info._lora_modulation_info._code_rate);
    } else {
        data.add("modu", "FSK");
        data.add("datr", frame._tx_info._fsk_modulation_info._data_rate);
    }
    data.add("rssi", frame._rx_info._rssi);
    data.add("lsnr", frame._rx_info._lora_snr);
    data.add("size", frame._phy_payload.size());
    data.add("data", base64_encode({frame._phy_payload.data(), frame._phy_payload.size()}));
    auto&& data_str = data.string();
    std::copy(data_str.begin(), data_str.end(), std::back_inserter(packet));

    // Return
    return packet;
}

bool gateway::is_push_ack(const byte* resp, size_t resp_len, const std::vector<byte>& packet) {
    if (resp_len != 4) {
        return false;
    }
    if (resp[0] != 0x02 || resp[1] != packet[1] || resp[2] != packet[2] || resp[3] != 0x01) {
        return false;
    }
    return true;
}

}
