//
// Created by chungphb on 25/5/21.
//

#include <supg/core/device.h>
#include <supg/util/helper.h>
#include <spdlog/spdlog.h>

namespace supg {

using namespace std::chrono_literals;

void device::run() {
    _uplink_loop = std::async(std::launch::async, &device::uplink_loop, this);
    _downlink_loop = std::async(std::launch::async, &device::downlink_loop, this);
}

void device::stop() {
    if (_downlink_frames) {
        _downlink_frames->close();
    }
    if (_uplink_loop.valid()) {
        _uplink_loop.get();
    }
    if (_downlink_loop.valid()) {
        _downlink_loop.get();
    }
}

void device::add_gateway(std::shared_ptr<gateway> gateway) {
    gateway->add_device(_dev_eui, _downlink_frames);
    _gateways.push_back(std::move(gateway));
}

void device::uplink_loop() {
    std::this_thread::sleep_for(5s);
    while (!_stopped) {
        send_data();
        std::this_thread::sleep_for(std::chrono::seconds(_uplink_interval));
    }
}

void device::send_data() {
    lora::phy_payload phy_payload{};

    // Set MAC header
    phy_payload._mhdr._m_type = lora::m_type::unconfirmed_data_up;
    phy_payload._mhdr._major = lora::major::lorawan_r1;

    // Set MAC payload
    phy_payload._mac_payload._fhdr._dev_addr = _dev_addr;
    phy_payload._mac_payload._fhdr._f_cnt = _f_cnt_up;
    phy_payload._mac_payload._fhdr._f_ctrl._adr = false;
    phy_payload._mac_payload._f_port = std::make_unique<uint8_t>(_f_port);
    lora::data_payload data;
    data._data = _payload;
    phy_payload._mac_payload._frm_payload.push_back(std::move(data));

    // Encrypt
    phy_payload.encrypt_frm_payload(_app_s_key);

    // Set MIC
    lora::phy_payload::uplink_data_info info{};
    info._m_ver = lora::mac_version::lorawan_1_0;
    info._conf_f_cnt = 0;
    info._tx_dr = 0;
    info._tx_ch = 0;
    info._f_nwk_s_int_key = _nwk_s_key;
    info._s_nwk_s_int_key = _nwk_s_key;
    phy_payload.set_uplink_data_mic(info);

    // Send payload
    spdlog::debug("DEV {}: Send uplink packet #{}", _dev_eui.string(), _f_cnt_up);
    ++_f_cnt_up;
    send_uplink(std::move(phy_payload));
}

void device::send_uplink(lora::phy_payload phy_payload) {
    // Prepare uplink frame
    gw::uplink_frame frame;
    frame._phy_payload = phy_payload.marshal();
    frame._tx_info = _uplink_tx_info;

    // Send uplink frame
    for (const auto& gateway : _gateways) {
        gateway->send_uplink_frame(frame);
    }
}

void device::downlink_loop() {
    gw::downlink_frame frame;
    while (!_stopped) {
        if (_downlink_frames->is_closed()) {
            break;
        }
        if (_downlink_frames->get(frame)) {
            auto payload = base64_decode({frame._phy_payload.data(), frame._phy_payload.size()});
            lora::phy_payload phy_payload{};
            try {
                phy_payload.unmarshal({payload.begin(), payload.end()});
            } catch (...) {
                spdlog::error("DEV {}: Handle unsupported payload", _dev_eui.string());
                break;
            }
            switch (phy_payload._mhdr._m_type) {
                case lora::m_type::join_accept: {
                    spdlog::error("DEV {}: Handle unsupported payload", _dev_eui.string());
                    break;
                }
                case lora::m_type::unconfirmed_data_down:
                case lora::m_type::confirmed_data_down: {
                    handle_data(std::move(phy_payload));
                    break;
                }
                default: {
                    throw std::runtime_error("Simulator: Something went wrong");
                }
            }
        }
        std::this_thread::sleep_for(50ms);
    }
}

void device::handle_data(lora::phy_payload phy_payload) {
    // Validate MIC
    lora::phy_payload::downlink_data_info data_info{};
    data_info._m_ver = lora::mac_version::lorawan_1_0;
    data_info._conf_f_cnt = 0;
    data_info._s_nwk_s_int_key = _nwk_s_key;
    try {
        auto res = phy_payload.validate_downlink_data_mic(data_info);
        if (!res) {
            spdlog::trace("DEV {}: Invalid MIC", _dev_eui.string());
            return;
        }
    } catch (...) {
        spdlog::trace("DEV {}: Invalid MIC", _dev_eui.string());
        return;
    }

    // Reset frame count
    _f_cnt_down += static_cast<uint16_t>(phy_payload._mac_payload._fhdr._f_cnt) - static_cast<uint16_t>(_f_cnt_down % (1 << 16));

    // Decrypt frame payload
    std::vector<byte> data;
    auto f_port = phy_payload._mac_payload._f_port ? *phy_payload._mac_payload._f_port : 0;
    if (f_port != 0) {
        phy_payload.decrypt_frm_payload(_app_s_key);
        if (!phy_payload._mac_payload._frm_payload.empty()) {
            data = phy_payload._mac_payload._frm_payload[0]._data;
        }
    }

    // Handle downlink data
    if (_downlink_data_handler) {
        downlink_data_info downlink_data_info;
        downlink_data_info._confirmed = phy_payload._mhdr._m_type == lora::m_type::confirmed_data_down;
        downlink_data_info._ack = phy_payload._mac_payload._fhdr._f_ctrl._ack;
        downlink_data_info._f_cnt_down = _f_cnt_down;
        downlink_data_info._f_port = f_port;
        downlink_data_info._data = std::move(data);
        _downlink_data_handler(std::move(downlink_data_info));
    }

    spdlog::debug("DEV {}: Handle {} #{}", _dev_eui.string(), f_port == 0 ? "MAC command" : "downlink packet", _f_cnt_down);
}

}