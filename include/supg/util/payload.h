//
// Created by chungphb on 24/5/21.
//

#pragma once

#include <string>
#include <vector>

namespace supg {

const std::vector<std::string> UPLINK_PAYLOAD_LIST = {
    R"({
        "rxpk": [
            {
                "time":"2021-01-21T16:21:17.530974Z",
                "tmst":3512348514,
                "chan":9,
                "rfch":1,
                "freq":869.1,
                "stat":1,
                "modu":"FSK",
                "datr":50000,
                "rssi":-75,
                "size":16,
                "data":"VEVTVF9QQUNLRVRfMTIzNA=="
            }
        ]
    })",
    R"({
        "rxpk": [
            {
                "time":"2021-01-21T16:21:17.532038Z",
		        "tmst":3316387610,
                "chan":0,
		        "rfch":0,
		        "freq":863.00981,
		        "stat":1,
		        "modu":"LORA",
		        "datr":"SF10BW125",
		        "codr":"4/7",
		        "rssi":-38,
		        "lsnr":5.5,
		        "size":32,
		        "data":"VEVTVF9QQUNLRVRfMTIzNA=="
            }
        ]
    })",
    R"({
        "rxpk": [
            {
                "time":"2021-01-21T16:21:17.538002Z",
                "tmst":3512348611,
                "chan":2,
                "rfch":0,
                "freq":866.349812,
                "stat":1,
                "modu":"LORA",
                "datr":"SF7BW125",
                "codr":"4/6",
                "rssi":-35,
                "lsnr":5.1,
                "size":32,
                "data":"VEVTVF9QQUNLRVRfMTIzNA=="
            }
        ]
    })"
};

}