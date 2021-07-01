//
// Created by vht on 7/1/21.
//

#pragma once

#include <supg/util/data_types.h>

namespace supg {
namespace gw {

// Uplink TX info

enum struct modulation : byte {
    lora = 0,
    fsk = 1
};

struct lora_modulation_info {
    uint32_t _bandwidth = 0;
    uint32_t _spreading_factor = 0;
    std::string _code_rate;
    bool _polarization_inversion = false;
};

struct fsk_modulation_info {
    uint32_t _frequency_deviation = 0;
    uint32_t _data_rate = 0;
};

struct uplink_tx_info {
    uint32_t _frequency = 0;
    modulation _modulation = modulation::lora;
    lora_modulation_info _lora_modulation_info;
    fsk_modulation_info _fsk_modulation_info;
};

// Uplink RX info

enum struct location_source : byte {
    unknown = 0,
    gps = 1,
    config = 2,
    geo_resolver_tdoa = 3,
    geo_resolver_rssi = 4,
    geo_resolver_gnss = 5,
    geo_resolver_wifi = 6
};

struct location {
    double _latitude = 0;
    double _longitude = 0;
    double _altitude = 0;
    location_source _source = location_source::unknown;
    uint32_t _accuracy = 0;
};

enum struct crc_status : byte {
    no_crc = 0,
    bad_crc = 1,
    crc_ok = 2
};

struct uplink_rx_info {
    std::vector<byte> _gateway_id;
    std::string _time;
    uint64_t _time_since_gps_epoch = 0;
    int32_t _rssi = 0;
    double _lora_snr = 0;
    uint32_t _channel = 0;
    uint32_t _rf_chain = 0;
    uint32_t _board = 0;
    uint32_t _antenna = 0;
    location _location;
    std::vector<byte> _context;
    std::vector<byte> _uplink_id;
    crc_status _crc_status = crc_status::crc_ok;
};

// Uplink frame

struct uplink_frame {
    std::vector<byte> _phy_payload;
    uplink_tx_info _tx_info;
    uplink_rx_info _rx_info;
};

}
}
