//
// Created by chungphb on 25/5/21.
//

#include <supg/core/device.h>
#include <spdlog/spdlog.h>

namespace supg {

void device::run() {
    send_data();
}

void device::stop() {
    _stopped = true;
}

void device::send_data() {
    lora::phy_payload phy_payload{};

    // Set MAC header
    phy_payload._mhdr._m_type = lora::m_type::unconfirmed_data_up;
    phy_payload._mhdr._major = lora::major::lorawan_r1;

    // Set MAC payload
    phy_payload._mac_payload._fhdr._dev_addr = _dev_addr;
    phy_payload._mac_payload._fhdr._f_cnt = _f_cnt;
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
    spdlog::debug("DEV {}: Send uplink packet #{}", _dev_eui.string(), _f_cnt);
    ++_f_cnt;
    send_uplink(std::move(phy_payload));
}

void device::send_uplink(lora::phy_payload&& phy_payload) {
    // Prepare uplink frame
    gw::uplink_frame frame;
    frame._phy_payload = phy_payload.marshal();
    frame._tx_info = _uplink_tx_info;

    // Send uplink frame
    for (const auto& gateway : _gateways) {
        gateway->send_uplink_frame(frame);
    }
}

}