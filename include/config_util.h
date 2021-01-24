/*****************************************************************
*   Copyright (C) 2018 . All rights reserved.
*
*   @file:    config_util.h
*   @author:  marinyoung@163.com
*   @date:    2018/12/28 11:50:33
*   @brief  
*
*****************************************************************/

#ifndef __CONFIG_UTIL_H_
#define __CONFIG_UTIL_H_

#include "common.h"
#include "str_util.h"
#include "file_util.h"
#include <string>
#include <unordered_map>

namespace tiny_engine {

class ConfigUtil {
public:
    ConfigUtil(const std::string &config = "./conf/sys.conf");

    bool init();
    std::string get_config_file() const;
    std::size_t get_config_size() const;

    bool get_value(const std::string &key, int32_t &value) const;
    bool get_value(const std::string &key, float &value) const;
    bool get_value(const std::string &key, double &value) const;
    bool get_value(const std::string &key, std::string &value) const;

private:
    DISALLOW_COPY_AND_ASSIGN(ConfigUtil);

private:
    std::string config_file;
    std::unordered_map<std::string, std::string> config_map;
};

}; // end of namespace tiny_engine
#endif // __CONFIG_UTIL_H_

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
