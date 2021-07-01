//
// Created by chungphb on 24/5/21.
//

#pragma once

#include <supg/util/data_types.h>
#include <memory>

namespace supg {
namespace lora {

// Data payload

struct eui64 {
    std::vector<byte> marshal();
    void unmarshal(const std::vector<byte>& data);
    std::basic_string<byte> string();
    bool operator<(const eui64& rhs) const;
    std::array<byte, 8> _value;
};

struct data_payload {
    std::vector<byte> marshal();
    void unmarshal(const std::vector<byte>& data, bool uplink);
    std::vector<byte> _data;
};

// MAC payload

struct dev_addr {
    std::vector<byte> marshal();
    void unmarshal(const std::vector<byte>& data);
    std::array<byte, 4> _value;
};

struct f_ctrl {
    std::vector<byte> marshal();
    void unmarshal(const std::vector<byte>& data);
    bool _adr, _adr_ack_req, _ack, _f_pending, _class_b;
    uint8_t _f_opts_len;
};

struct fhdr {
    std::vector<byte> marshal();
    void unmarshal(const std::vector<byte>& data);
    dev_addr _dev_addr;
    f_ctrl _f_ctrl;
    uint32_t _f_cnt;
    std::vector<data_payload> _f_opts;
};

struct mac_payload {
    std::vector<byte> marshal();
    void unmarshal(const std::vector<byte>& data, bool uplink);
    std::vector<byte> marshal_payload();
    fhdr _fhdr;
    std::unique_ptr<uint8_t> _f_port;
    std::vector<data_payload> _frm_payload;
};

// Physical payload

enum struct m_type : byte {
    join_request = 0,
    join_accept = 1,
    unconfirmed_data_up = 2,
    unconfirmed_data_down = 3,
    confirmed_data_up = 4,
    confirmed_data_down = 5,
    rejoin_request = 6,
    proprietary = 7
};

enum struct major : byte {
    lorawan_r1 = 0
};

enum struct mac_version : byte {
    lorawan_1_0 = 0,
    lorawan_1_1 = 1
};

struct aes128key {
    std::vector<byte> marshal();
    void unmarshal(const std::vector<byte>& data);
    std::basic_string<byte> string();
    std::array<byte, 16> _value;
};

struct mhdr {
    std::vector<byte> marshal();
    void unmarshal(const std::vector<byte>& data);
    m_type _m_type;
    major _major;
};

struct mic {
    bool operator==(const mic& rhs) const;
    bool operator!=(const mic& rhs) const;
    std::array<byte, 4> _value;
};

struct phy_payload {
    struct uplink_data_info {
        mac_version _m_ver;
        uint32_t _conf_f_cnt;
        uint8_t _tx_dr;
        uint8_t _tx_ch;
        aes128key _f_nwk_s_int_key;
        aes128key _s_nwk_s_int_key;
    };

    struct downlink_data_info {
        mac_version _m_ver;
        uint32_t _conf_f_cnt;
        aes128key _s_nwk_s_int_key;
    };

    struct frm_payload_info {
        aes128key _key;
        bool _is_uplink;
        dev_addr _dev_addr;
        uint32_t _f_cnt;
        std::vector<byte> _data;
    };

    // For uplink data frame
    void set_uplink_data_mic(uplink_data_info& info);
    bool validate_uplink_data_mic(uplink_data_info& info);
    bool validate_uplink_data_micf(aes128key f_nwk_s_int_key);
    mic calculate_uplink_data_mic(uplink_data_info& info);

    // For downlink data frame
    void set_downlink_data_mic(downlink_data_info& info);
    bool validate_downlink_data_mic(downlink_data_info& info);
    mic calculate_downlink_data_mic(downlink_data_info& info);

    // For frame payload
    void encrypt_frm_payload(aes128key key);
    void decrypt_frm_payload(aes128key key);
    std::vector<byte> encrypt_frm_payload(frm_payload_info& info);

    // For physical payload
    std::vector<byte> marshal();
    void unmarshal(const std::vector<byte>& data);
    bool is_uplink();

    mhdr _mhdr;
    mac_payload _mac_payload;
    mic _mic;
};

}
}