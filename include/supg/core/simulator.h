//
// Created by chungphb on 25/5/21.
//

#pragma once

#include <supg/core/device.h>
#include <supg/core/gateway.h>
#include <supg/core/config.h>

namespace supg {

struct simulator {
public:
    void generate_config_file(const std::string& config_file);
    void set_config_file(const std::string& config_file);
    void init();
    void run();
    void stop();

private:
    bool is_running();

private:
    std::string _config_file;
    config _config;
    std::vector<std::shared_ptr<device>> _dev_list;
    std::vector<std::shared_ptr<gateway>> _gw_list;
    bool _stopped = false;
};

}