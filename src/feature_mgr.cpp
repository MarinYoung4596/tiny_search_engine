/*****************************************************************
*   Copyright (C) 2020  Inc. All rights reserved.
*
*   @file:    feature_mgr.cpp
*   @author:  marinyoung@163.com
*   @date:    2020/08/30 13:25:12
*   @brief:   
*
*****************************************************************/


#include "../include/feature_mgr.h"
#include "../third_party/jsonxx/json.hpp"

namespace tiny_engine {

void FeatureMgr::init() {
    name_value_map.clear();
}

std::string FeatureMgr::to_str() const {
    jsonxx::json j;
    for (auto it = name_value_map.begin(); it != name_value_map.end(); ++it) {
        j[it->first] = it->second;
    }
    return j.dump();
}

bool FeatureMgr::is_empty() const {
    return name_value_map.empty();
}

std::unordered_map<std::string, float> FeatureMgr::get_features() const {
    return name_value_map;
}

float FeatureMgr::get_feature(const std::string &name, float default_value) const {
    auto iter = name_value_map.find(name);
    if (iter != name_value_map.end()) {
        return iter->second;
    }
    return default_value;
}

}; // end of namespace tiny_engine

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
