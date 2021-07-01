//
// Created by chungphb on 25/5/21.
//

#include <supg/core/payload.h>
#include <supg/util/helper.h>
#include <cmac.h>
#include <aes.h>
#include <modes.h>
#include <filters.h>
#include <algorithm>
#include <iostream>

namespace supg {
namespace lora {

std::vector<byte> eui64::marshal() {
    std::vector<byte> res{_value.rbegin(), _value.rend()};
    return res;
}

void eui64::unmarshal(const std::vector<byte>& data) {
    if (data.size() != _value.size()) {
        throw std::runtime_error("lora: failed to unmarshal eui64");
    }
    for (int i = 0; i < _value.size(); ++i) {
        _value[_value.size() - (i + 1)] = data[i];
    }
}

std::basic_string<byte> eui64::string() {
    return to_hex_string(_value.data(), _value.size());
}

bool eui64::operator<(const eui64& rhs) const {
    for (int i = 0; i < _value.size(); ++i) {
        if (_value[i] < rhs._value[i]) {
            return true;
        }
    }
    return false;
}

std::vector<byte> data_payload::marshal() {
    return _data;
}

void data_payload::unmarshal(const std::vector<byte>& data, bool uplink) {
    _data = data;
}

std::vector<byte> dev_addr::marshal() {
    std::vector<byte> res{_value.rbegin(), _value.rend()};
    return res;
}

void dev_addr::unmarshal(const std::vector<byte>& data) {
    if (data.size() != _value.size()) {
        throw std::runtime_error("lora: failed to unmarshal device address");
    }
    for (int i = 0; i < _value.size(); ++i) {
        _value[_value.size() - (i + 1)] = data[i];
    }
}

std::vector<byte> f_ctrl::marshal() {
    if (_f_opts_len > 15) {
        throw std::runtime_error("lora: max value of f_opts_len is 15");
    }
    byte res = 0;
    if (_adr) {
        res |= static_cast<byte>(0x80);
    }
    if (_adr_ack_req) {
        res |= static_cast<byte>(0x40);
    }
    if (_ack) {
        res |= static_cast<byte>(0x20);
    }
    if (_class_b || _f_pending) {
        res |= static_cast<byte>(0x10);
    }
    res |= static_cast<byte>(_f_opts_len & 0x0f);
    return {res};
}

void f_ctrl::unmarshal(const std::vector<byte>& data) {
    if (data.size() != 1) {
        throw std::runtime_error("lora: failed to unmarshal frame control");
    }
    _adr = (data[0] & 0x80) != 0;
    _adr_ack_req = (data[0] & 0x40) != 0;
    _ack = (data[0] & 0x20) != 0;
    _class_b = (data[0] & 0x10) != 0;
    _f_pending = (data[0] & 0x10) != 0;
    _f_opts_len = data[0] & 0x0f;
}

std::vector<byte> fhdr::marshal() {
    std::vector<byte> res;
    std::vector<byte> bytes;

    // Marshal frame options
    std::vector<byte> f_opts_bytes;
    for (auto& mac : _f_opts) {
        bytes = mac.marshal();
        f_opts_bytes.insert(f_opts_bytes.end(), bytes.begin(), bytes.end());
    }
    _f_ctrl._f_opts_len = static_cast<uint8_t>(f_opts_bytes.size());
    if (_f_ctrl._f_opts_len > 15) {
        throw std::runtime_error("lora: max value of f_opts_len is 15");
    }
    res.reserve(7 + _f_ctrl._f_opts_len);

    // Marshal device address
    bytes = _dev_addr.marshal();
    res.insert(res.end(), bytes.begin(), bytes.end());

    // Marshal frame control
    bytes = _f_ctrl.marshal();
    res.insert(res.end(), bytes.begin(), bytes.end());

    // Marshal frame count
    std::array<byte, sizeof(_f_cnt)> f_cnt_bytes{};
    std::memcpy(f_cnt_bytes.data(), &_f_cnt, sizeof(_f_cnt));
    res.insert(res.end(), f_cnt_bytes.begin(), f_cnt_bytes.begin() + 2);

    // Append frame options and return
    res.insert(res.end(), f_opts_bytes.begin(), f_opts_bytes.end());
    return res;
}

void fhdr::unmarshal(const std::vector<byte>& data) {
    if (data.size() < 7) {
        throw std::runtime_error("lora: failed to unmarshal frame header");
    }
    std::vector<byte> bytes;

    // Unmarshal device address
    bytes = {data.begin(), data.begin() + 4};
    _dev_addr.unmarshal(bytes);

    // Unmarshal frame control
    bytes = {data.begin() + 4, data.begin() + 5};
    _f_ctrl.unmarshal(bytes);

    // Unmarshal frame count
    bytes = {data.begin() + 5, data.begin() + 7};
    bytes.push_back(0x00);
    bytes.push_back(0x00);
    std::memcpy(&_f_cnt, bytes.data(), sizeof(_f_cnt));

    // Unmarshal frame options
    if (data.size() > 7) {
        bytes = {data.begin() + 7, data.end()};
        data_payload payload;
        payload._data = std::move(bytes);
        _f_opts.push_back(std::move(payload));
    }
}

std::vector<byte> mac_payload::marshal() {
    std::vector<byte> res;
    std::vector<byte> bytes;

    // Marshal frame header
    bytes = _fhdr.marshal();
    res.insert(res.end(), bytes.begin(), bytes.end());

    // Marshal frame port
    if (!_f_port) {
        if (!_frm_payload.empty()) {
            throw std::runtime_error("lora: frm_payload is not empty");
        }
        return res;
    }
    if (!_fhdr._f_opts.empty() && *_f_port == 0) {
        throw std::runtime_error("lora: f_opts are set");
    }
    res.push_back(static_cast<byte>(*_f_port));

    // Marshal payloads
    bytes = marshal_payload();
    res.insert(res.end(), bytes.begin(), bytes.end());

    // Return
    return res;
}

void mac_payload::unmarshal(const std::vector<byte>& data, bool uplink) {
    if (data.size() < 7) {
        throw std::runtime_error("lora: failed to unmarshal mac payload");
    }
    std::vector<byte> bytes;

    // Unmarshal frame control to continue checking size
    bytes = {data.begin() + 4, data.begin() + 5};
    _fhdr._f_ctrl.unmarshal(bytes);

    if (data.size() < 7 + _fhdr._f_ctrl._f_opts_len) {
        throw std::runtime_error("lora: failed to unmarshal mac payload");
    }

    // Unmarshal frame header
    bytes = {data.begin(), data.begin() + 7 + _fhdr._f_ctrl._f_opts_len};
    _fhdr.unmarshal(bytes);

    // Unmarshal frame port
    if (data.size() > 7 + _fhdr._f_ctrl._f_opts_len) {
        _f_port = std::make_unique<uint8_t>(data[7 + _fhdr._f_ctrl._f_opts_len]);
    }

    // Unmarshal frame payloads
    if (data.size() > 7 + _fhdr._f_ctrl._f_opts_len + 1) {
        if (_f_port && *_f_port == 0 && _fhdr._f_ctrl._f_opts_len > 0) {
            throw std::runtime_error("lora: f_opts are set");
        }
        bytes = {data.begin() + 7 + _fhdr._f_ctrl._f_opts_len + 1, data.end()};
        data_payload payload;
        payload._data = std::move(bytes);
        _frm_payload.push_back(std::move(payload));
    }
}

std::vector<byte> mac_payload::marshal_payload() {
    std::vector<byte> res;
    std::vector<byte> bytes;
    for (auto& payload : _frm_payload) {
        bytes = payload.marshal();
        res.insert(res.end(), bytes.begin(), bytes.end());
    }
    return res;
}

std::vector<byte> aes128key::marshal() {
    std::vector<byte> res{_value.rbegin(), _value.rend()};
    return res;
}

void aes128key::unmarshal(const std::vector<byte>& data) {
    if (data.size() != _value.size()) {
        throw std::runtime_error("lora: failed to unmarshal eui64");
    }
    for (int i = 0; i < _value.size(); ++i) {
        _value[_value.size() - (i + 1)] = data[i];
    }
}

std::basic_string<byte> aes128key::string() {
    return to_hex_string(_value.data(), _value.size());
}

std::vector<byte> mhdr::marshal() {
    byte res = (static_cast<byte>(_m_type) << 5) | (static_cast<byte>(_major) & 0x03);
    return {res};
}

void mhdr::unmarshal(const std::vector<byte>& data) {
    if (data.size() != 1) {
        throw std::runtime_error("lora: failed to unmarshal mac header");
    }
    _m_type = static_cast<m_type>(data[0] >> 5);
    _major = static_cast<major>(data[0] & 0x03);
}

bool mic::operator==(const mic& rhs) const {
    for (int i = 0; i < _value.size(); ++i) {
        if (_value[i] != rhs._value[i]) {
            return false;
        }
    }
    return true;
}

bool mic::operator!=(const mic& rhs) const {
    return !operator==(rhs);
}

void phy_payload::set_uplink_data_mic(uplink_data_info& info) {
    _mic = calculate_uplink_data_mic(info);
}

bool phy_payload::validate_uplink_data_mic(uplink_data_info& info) {
    auto mic = calculate_uplink_data_mic(info);
    return _mic == mic;
}

bool phy_payload::validate_uplink_data_micf(aes128key f_nwk_s_int_key) {
    // Calculate mic
    uplink_data_info info{};
    info._m_ver = mac_version::lorawan_1_1;
    info._conf_f_cnt = 0;
    info._tx_ch = 0;
    info._tx_dr = 0;
    info._f_nwk_s_int_key = f_nwk_s_int_key;
    info._s_nwk_s_int_key = f_nwk_s_int_key;
    auto mic = calculate_uplink_data_mic(info);

    // Validate micf
    for (int i = 2; i < _mic._value.size(); ++i) {
        if (_mic._value[i] != mic._value[i]) {
            return false;
        }
    }
    return true;
}

mic phy_payload::calculate_uplink_data_mic(uplink_data_info& info) {
    mic mic{};
    std::vector<byte> plain;
    std::vector<byte> bytes;

    // Reset frame count
    info._conf_f_cnt = _mac_payload._fhdr._f_ctrl._ack ? (info._conf_f_cnt % (1 << 16)) : 0;

    // Marshal MAC header
    bytes = _mhdr.marshal();
    plain.insert(plain.end(), bytes.begin(), bytes.end());

    // Marshal MAC payload
    bytes = _mac_payload.marshal();
    plain.insert(plain.end(), bytes.begin(), bytes.end());

    // Marshal other info
    std::vector<byte> bytes_0(16, 0x00);
    std::vector<byte> bytes_1(16, 0x00);
    bytes_0[0] = 0x49;
    bytes_1[0] = 0x49;

    bytes = _mac_payload._fhdr._dev_addr.marshal();
    std::copy(bytes.begin(), bytes.end(), bytes_0.begin() + 6);
    std::copy(bytes.begin(), bytes.end(), bytes_1.begin() + 6);

    auto f_cnt = _mac_payload._fhdr._f_cnt;
    std::memcpy(bytes_0.data() + 10, &f_cnt, sizeof(f_cnt));
    std::memcpy(bytes_1.data() + 10, &f_cnt, sizeof(f_cnt));

    bytes_0[15] = static_cast<byte>(plain.size());
    bytes_1[15] = static_cast<byte>(plain.size());

    auto conf_f_cnt = static_cast<uint16_t>(info._conf_f_cnt);
    std::memcpy(bytes_1.data() + 1, &conf_f_cnt, sizeof(conf_f_cnt));
    bytes_1[3] = static_cast<byte>(info._tx_dr);
    bytes_1[4] = static_cast<byte>(info._tx_ch);

    // Encode
    std::vector<byte> f_encoded;
    std::vector<byte> s_encoded;

    // Encode second half
    CryptoPP::SecByteBlock key{CryptoPP::AES::DEFAULT_KEYLENGTH};
    for (int i = 0; i < key.size(); ++i) {
        key[i] = (CryptoPP::byte)info._s_nwk_s_int_key._value[i];
    }
    try {
        CryptoPP::CMAC<CryptoPP::AES> cmac{key.data(), key.size()};
        cmac.Update((const CryptoPP::byte*)bytes_1.data(), bytes_1.size());
        cmac.Update((const CryptoPP::byte*)plain.data(), plain.size());
        s_encoded.resize(cmac.DigestSize());
        cmac.Final((CryptoPP::byte*)&s_encoded[0]);
    } catch (const CryptoPP::Exception& ex) {
        std::cerr << "lora: failed to encrypt: " << ex.what() << '\n';
    }
    if (s_encoded.size() < 4) {
        throw std::runtime_error("lora: the hash returned less than 4 bytes");
    }

    // Encode first half
    for (int i = 0; i < key.size(); ++i) {
        key[i] = (CryptoPP::byte)info._f_nwk_s_int_key._value[i];
    }
    try {
        CryptoPP::CMAC<CryptoPP::AES> cmac{key.data(), key.size()};
        cmac.Update((const CryptoPP::byte*)bytes_0.data(), bytes_0.size());
        cmac.Update((const CryptoPP::byte*)plain.data(), plain.size());
        f_encoded.resize(cmac.DigestSize());
        cmac.Final((CryptoPP::byte*)&f_encoded[0]);
    } catch (const CryptoPP::Exception& ex) {
        std::cerr << "lora: failed to encrypt: " << ex.what() << '\n';
    }
    if (f_encoded.size() < 2) {
        throw std::runtime_error("lora: the hash returned less than 2 bytes");
    }

    // Generate MIC
    switch (info._m_ver) {
        case mac_version::lorawan_1_0: {
            std::copy_n(f_encoded.begin(), 4, mic._value.begin());
            break;
        }
        case mac_version::lorawan_1_1: {
            std::copy_n(s_encoded.begin(), 2, mic._value.begin());
            std::copy_n(f_encoded.begin(), 2, mic._value.begin() + 2);
            break;
        }
    }
    return mic;
}

void phy_payload::set_downlink_data_mic(downlink_data_info& info) {
    _mic = calculate_downlink_data_mic(info);
}

bool phy_payload::validate_downlink_data_mic(downlink_data_info& info) {
    auto mic = calculate_downlink_data_mic(info);
    return _mic == mic;
}

mic phy_payload::calculate_downlink_data_mic(downlink_data_info& info) {
    mic mic{};
    std::vector<byte> encoded;
    std::vector<byte> plain;
    std::vector<byte> bytes;

    // Reset frame count
    if (info._m_ver == mac_version::lorawan_1_0 || !_mac_payload._fhdr._f_ctrl._ack) {
        info._conf_f_cnt = 0;
    }
    info._conf_f_cnt %= (1 << 16);

    // Marshal MAC header
    bytes = _mhdr.marshal();
    plain.insert(plain.end(), bytes.begin(), bytes.end());

    // Marshal MAC payload
    bytes = _mac_payload.marshal();
    plain.insert(plain.end(), bytes.begin(), bytes.end());

    // Marshal other info
    std::vector<byte> bytes_0(16, 0x00);
    bytes_0[0] = 0x49;

    auto conf_f_cnt = static_cast<uint16_t>(info._conf_f_cnt);
    std::memcpy(bytes_0.data() + 1, &conf_f_cnt, sizeof(conf_f_cnt));

    bytes_0[5] = 0x01;

    bytes = _mac_payload._fhdr._dev_addr.marshal();
    std::copy(bytes.begin(), bytes.end(), bytes_0.begin() + 6);

    auto f_cnt = _mac_payload._fhdr._f_cnt;
    std::memcpy(bytes_0.data() + 10, &f_cnt, sizeof(f_cnt));

    bytes_0[15] = static_cast<byte>(plain.size());

    // Encode
    CryptoPP::SecByteBlock key{CryptoPP::AES::DEFAULT_KEYLENGTH};
    for (int i = 0; i < key.size(); ++i) {
        key[i] = (CryptoPP::byte)info._s_nwk_s_int_key._value[i];
    }
    try {
        CryptoPP::CMAC<CryptoPP::AES> cmac{key.data(), key.size()};
        cmac.Update((const CryptoPP::byte*)bytes_0.data(), bytes_0.size());
        cmac.Update((const CryptoPP::byte*)plain.data(), plain.size());
        encoded.resize(cmac.DigestSize());
        cmac.Final((CryptoPP::byte*)&encoded[0]);
    } catch (const CryptoPP::Exception& ex) {
        std::cerr << "lora: failed to encrypt: " << ex.what() << '\n';
    }
    if (encoded.size() < 4) {
        throw std::runtime_error("lora: the hash returned less than 4 bytes");
    }

    // Generate MIC
    std::copy_n(encoded.begin(), 4, mic._value.begin());
    return mic;
}

void phy_payload::encrypt_frm_payload(aes128key key) {
    if (_mac_payload._frm_payload.empty()) {
        return;
    }
    std::vector<byte> data;

    // Prepare data
    data = _mac_payload.marshal_payload();

    // Encrypt data
    frm_payload_info info;
    info._key = key;
    info._is_uplink = is_uplink();
    info._dev_addr = _mac_payload._fhdr._dev_addr;
    info._f_cnt = _mac_payload._fhdr._f_cnt;
    info._data = std::move(data);
    data = encrypt_frm_payload(info);

    // Store data
    data_payload frm_payload;
    frm_payload._data = std::move(data);
    _mac_payload._frm_payload.clear();
    _mac_payload._frm_payload.push_back(std::move(frm_payload));
}

void phy_payload::decrypt_frm_payload(aes128key key) {
    encrypt_frm_payload(key);
    if (_mac_payload._f_port && *_mac_payload._f_port == 0) {
        throw std::runtime_error("lora: mac commands are not supported");
    }
}

std::vector<byte> phy_payload::encrypt_frm_payload(frm_payload_info& info) {
    // Add padding
    auto data_len = info._data.size();
    if (data_len % 16) {
        std::vector<byte> padding(16 - data_len % 16, 0x00);
        info._data.insert(info._data.end(), padding.begin(), padding.end());
    }

    // Prepare data
    std::string plain(16, 0x00);
    plain[0] = 0x01;
    if (!info._is_uplink) {
        plain[5] = 0x01;
    }
    auto bytes = info._dev_addr.marshal();
    std::copy(bytes.begin(), bytes.end(), plain.begin() + 6);
    std::memcpy(&plain[10], &info._f_cnt, sizeof(info._f_cnt));

    // Encrypt data
    CryptoPP::ECB_Mode<CryptoPP::AES>::Encryption encryption;
    encryption.SetKey((const CryptoPP::byte*)info._key._value.data(), CryptoPP::AES::DEFAULT_KEYLENGTH);
    for (int i = 0; i < info._data.size() / 16; i++) {
        plain[15] = static_cast<byte>(i + 1);
        std::string cipher;
        CryptoPP::StreamTransformationFilter encryptor{encryption, new CryptoPP::StringSink(cipher)};
        encryptor.Put((const CryptoPP::byte*)plain.data(), plain.size());
        for (int j = 0; j < cipher.size(); ++j) {
            info._data[i * 16 + j] ^= cipher[j];
        }
    }

    // Return data
    return {info._data.begin(), info._data.begin() + data_len};
}

std::vector<byte> phy_payload::marshal() {
    std::vector<byte> res;
    std::vector<byte> bytes;

    // Marshal MAC header
    bytes = _mhdr.marshal();
    res.insert(res.end(), bytes.begin(), bytes.end());

    // Marshal MAC payload
    bytes = _mac_payload.marshal();
    res.insert(res.end(), bytes.begin(), bytes.end());

    // Marshal MIC
    res.insert(res.end(), _mic._value.begin(), _mic._value.end());

    // Return
    return res;
}

void phy_payload::unmarshal(const std::vector<byte>& data) {
    if (data.size() < 5) {
        throw std::runtime_error("lora: failed to unmarshal physical payload");
    }
    std::vector<byte> bytes;

    // Unmarshal MAC header
    bytes = {data.begin(), data.begin() + 1};
    _mhdr.unmarshal(bytes);

    // Unmarshal MAC payload
    bytes = {data.begin() + 1, data.begin() + data.size() - 4};
    switch (_mhdr._m_type) {
        case m_type::join_request:
        case m_type::join_accept:
        case m_type::rejoin_request:
        case m_type::proprietary: {
            std::cerr << "lora: message type not supported\n";
            break;
        }
        default: {
            _mac_payload.unmarshal(bytes, is_uplink());
            break;
        }
    }

    // Unmarshal MIC
    for (int i = 0; i < 4; ++i) {
        _mic._value[i] = data[data.size() - 4 + i];
    }
}

bool phy_payload::is_uplink() {
    switch (_mhdr._m_type) {
        case m_type::join_request:
        case m_type::unconfirmed_data_up:
        case m_type::confirmed_data_up:
        case m_type::rejoin_request: {
            return true;
        }
        default: {
            return false;
        }
    }
}

}
}
