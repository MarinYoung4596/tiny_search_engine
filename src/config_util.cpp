/*****************************************************************
*   Copyright (C) 2018 . All rights reserved.
*
*   @file:    config_util.cpp
*   @author:  marinyoung@163.com
*   @date:    2018/12/28 11:50:42
*   @brief:   
*
*****************************************************************/


#include "../include/config_util.h"

namespace tiny_engine {

ConfigUtil::ConfigUtil(const std::string &config) : config_file(config) {}

bool ConfigUtil::get_value(const std::string &key, std::string &value) const {
    EXPECT_FALSE_OR_RETURN(key.empty(), false);
    auto iter = config_map.find(key);
    if (iter == config_map.end()) {
        return false;
    }
    value = iter->second;
    return true;
}

bool ConfigUtil::get_value(const std::string &key, bool &value) const {
    EXPECT_FALSE_OR_RETURN(key.empty(), false);
    std::string str_value;
    EXPECT_TRUE_OR_RETURN(get_value(key, str_value), false);
    try {
        value = static_cast<bool>(std::stoi(str_value));
    } catch (std::exception &e) {
        return false;
    }
    return true;
}

bool ConfigUtil::get_value(const std::string &key, int &value) const {
    EXPECT_FALSE_OR_RETURN(key.empty(), false);
    std::string str_value;
    EXPECT_TRUE_OR_RETURN(get_value(key, str_value), false);
    try {
        value = std::stoi(str_value);
    } catch (std::exception &e) {
        return false;
    }
    return true;
}

bool ConfigUtil::get_value(const std::string &key, float &value) const {
    EXPECT_FALSE_OR_RETURN(key.empty(), false);
    std::string str_value;
    EXPECT_TRUE_OR_RETURN(get_value(key, str_value), false);
    try {
        value = std::stof(str_value);
    } catch (std::exception &e) {
        return false;
    }
    return true;
}

bool ConfigUtil::get_value(const std::string &key, double &value) const {
    EXPECT_FALSE_OR_RETURN(key.empty(), false);
    std::string str_value;
    EXPECT_TRUE_OR_RETURN(get_value(key, str_value), false);
    try {
        value = std::stod(str_value);
    } catch (std::exception &e) {
        return false;
    }
    return true;
}

bool ConfigUtil::init() {
    EXPECT_FALSE_OR_RETURN(config_file.empty(), false);
    EXPECT_TRUE_OR_RETURN(FileUtil::is_readable(config_file), false);
    std::ifstream ifs(config_file, std::ios::in);
    EXPECT_TRUE_OR_RETURN(ifs.is_open(), false);
    std::vector<std::string> vec;
    std::string line;
    while (!ifs.eof()) {
        std::getline(ifs, line);
        if (line.empty() || StrUtil::is_start_with(line, "#")) {
            continue;
        }
        StrUtil::split(line, ':', vec);
        if (vec.size() != 2) {
            continue;
        }
        auto key = StrUtil::trim(vec[0]);
        if (!StrUtil::is_upper_str(key)) {
            continue;
        }
        if (StrUtil::is_digital_str(key)) {
            continue;
        }
        auto value = StrUtil::trim(vec[1]);
        config_map[key] = value;
    }
    return true;
}

std::string ConfigUtil::get_config_file() const {
    return config_file;
}

std::size_t ConfigUtil::get_config_size() const {
    return config_map.size();
}

}; // end of namespace tiny_engine
/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
