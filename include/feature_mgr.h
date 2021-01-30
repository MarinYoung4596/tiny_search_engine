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

#include <unordered_map>
#include <string>

namespace tiny_engine {
class FeatureMgr {
public:
    FeatureMgr() {}

    void init();

    // only suitable for float, int, double, Type
    template <typename Type>
    void add_feature(const std::string &str, Type value);

    template <typename Type>
    void add_feature(const std::string &&str, Type value);

    std::string to_str() const;

private:
    std::unordered_map<std::string, float> name_value_map; // fname : fvalue
};


template <typename Type>
void FeatureMgr::add_feature(const std::string &str, Type value) {
    name_value_map[str] = static_cast<float>(value);
}

template <typename Type>
void FeatureMgr::add_feature(const std::string &&str, Type value) {
    name_value_map[str] = static_cast<float>(value);
}

}; // end of namespace tiny_engine
#endif // __FEATURE_MGR_H_
/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
