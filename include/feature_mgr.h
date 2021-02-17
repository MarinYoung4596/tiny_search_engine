/*****************************************************************
*   Copyright (C) 2020  Inc. All rights reserved.
*
*   @file:    feature_mgr.h
*   @author:  marinyoung@163.com
*   @date:    2020/08/30 13:25:00
*   @brief:   
*
*****************************************************************/

#ifndef __FEATURE_MGR_H_
#define __FEATURE_MGR_H_
#pragma once

#include "math_util.h"
#include "log_util.h"
#include <unordered_map>
#include <string>

namespace tiny_engine {
class FeatureMgr {
public:
    FeatureMgr() {}

    void init();

    // only suitable for float, int, double, Type
    template <typename Type>
    void add_feature(const std::string &key, Type value);
    
    template <typename Type>
    void add_feature(const std::string &&key, Type value);

    template <typename Type>
    bool get_feature(const std::string &key, Type &value) const;

    template <typename Type>
    bool get_feature(const std::string &&key, Type &value) const;

    std::string to_str() const;
    bool is_empty() const;

public:
    std::unordered_map<std::string, float> name_value_map; // fname : fvalue
};

template <typename Type>
void FeatureMgr::add_feature(const std::string &key, Type value) {
    name_value_map[key] = static_cast<float>(MathUtil::round(value, 3));
}

template <typename Type>
void FeatureMgr::add_feature(const std::string &&key, Type value) {
    name_value_map[key] = static_cast<float>(MathUtil::round(value, 3));
}

template <typename Type>
bool FeatureMgr::get_feature(const std::string &key, Type &value) const {
    auto iter = name_value_map.find(key);
    if (iter != name_value_map.end()) {
        value = iter->second;
        return true;
    }
    return false;
}

template <typename Type>
bool FeatureMgr::get_feature(const std::string &&key, Type &value) const {
    return get_feature(key, value);
}

}; // end of namespace tiny_engine
#endif // __FEATURE_MGR_H_
/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
