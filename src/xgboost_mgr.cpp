/*****************************************************************
*   Copyright (C) 2021  Inc. All rights reserved.
*
*   @file:    xgboost_mgr.cpp
*   @author:  marinyoung@163.com
*   @date:    2021/01/28 23:31:26
*   @brief:   
*
*****************************************************************/


#include "../include/xgboost_mgr.h"
#include <fstream>

namespace tiny_engine {

XGBoostMgr::XGBoostMgr(
        const string &model,
        const string &feature,
        float missing) :
        xgb_handler(nullptr),
        missing_value(missing),
        model_bin_path(model),
        feature_map_path(feature) {}

XGBoostMgr::~XGBoostMgr() {
    XGBoosterFree(xgb_handler);
}

bool XGBoostMgr::init() {
    feature_map.clear();
    return load_feature_map(feature_map_path) && load_model(model_bin_path);
}

bool XGBoostMgr::init(std::shared_ptr<ConfigUtil> configs) {
    EXPECT_NE_OR_RETURN(nullptr, configs, false);
    std::string value;
    if (!configs->get_value("MODEL_BIN", value)) {
        value = "./dict/xgb.ltr.model";
        LOG_WARNING("key[XGB_MODEL_BIN] not found, set to[%s]", value.c_str());
    }
    model_bin_path = value;
    if (!configs->get_value("FEATURE_MAP", value)) {
        value = "./conf/feature.conf";
        LOG_WARNING("key[FEATURE_MAP] not found, set to[%s]", value.c_str());
    }
    feature_map_path = value;
    configs->get_value("MISSING_VALUE", missing_value);
    return init();
}

bool XGBoostMgr::load_feature_map(const string &fpath) {
    EXPECT_FALSE_OR_RETURN(fpath.empty(), false);
    if (!feature_map.empty()) {
        feature_map.clear();
    }
    if (!FileUtil::is_readable(fpath)) {
        LOG_WARNING("file[%s] is not exist/readable", fpath.c_str());
        return false;
    }
    std::ifstream ifs(fpath);
    std::string line;
    std::vector<std::string> arr;
    auto index = 0;
    while (std::getline(ifs, line)) {
        if (StrUtil::is_start_with(line, "#")) {
            continue;
        }
        StrUtil::split(line, '\t', arr);
        if (arr.size() < 2) {
            continue;
        }
        auto fid = index;
        // auto fid = std::stoi(arr[0]);
        auto &fname = arr[1];
        feature_map[fname] = fid;
        index += 1;
    }
    LOG_INFO("load %lu items from %s, feature_map[\n%s\n]",
             feature_map.size(), fpath.c_str(),
             to_str().c_str());
    return true;
}

string XGBoostMgr::to_str() const {
    vector<string> items;
    for (const auto &item : feature_map) {
        items.push_back(StrUtil::format("{}:{}", item.second, item.first));
    }
    return StrUtil::join(items.begin(), items.end(), '\n');
}

bool XGBoostMgr::load_model(const string &fpath) {
    EXPECT_FALSE_OR_RETURN(fpath.empty(), false);
    if (!FileUtil::is_readable(fpath)) {
        LOG_WARNING("file[%s] is not exist/readable", fpath.c_str());
        return false;
    }

    auto ret1 = XGBoosterCreate(nullptr, 0, &xgb_handler);
    EXPECT_EQ_OR_RETURN_LOGGED(ret1, 0, false, "failed to create xgboost handler");
    auto ret2 = XGBoosterLoadModel(xgb_handler, fpath.c_str());
    if (0 != ret2) {
        LOG_ERROR("failed to load model from %s, ret=%d", fpath.c_str(), ret2);
        xgb_handler = nullptr;
        return false;
    }
    LOG_INFO("load xgboost model from %s ok", fpath.c_str());
    return true;
}

void XGBoostMgr::trans_to_svm_data(
        const unordered_map<string, float> &features,
        unordered_map<int, float> &out) const {
    if (!out.empty()) {
        out.clear();
    }
    EXPECT_FALSE_OR_RETURN(features.empty(), RETURN_ON_VOID);

    for (auto it = features.begin(); it != features.end(); ++it) {
        auto &fname = it->first;
        auto iter = feature_map.find(fname);
        if (iter == feature_map.end()) {
            continue;
        }
        auto fvalue = it->second;
        auto fid = iter->second;
        out[fid] = fvalue;
    }
}

bool XGBoostMgr::trans_to_dmatrix(
        const vector<unordered_map<string, float>> &features,
        DMatrixHandle &out) const {
    EXPECT_FALSE_OR_RETURN_LOGGED(features.empty(), false, "input feature empty!");
    const auto nrow = features.size();
    const auto ncolumn = feature_map.size();
    float data[nrow][ncolumn];
    memset(data, missing_value, sizeof(data));
    for (decltype(features.size()) i = 0; i < nrow; ++i) {
        const auto &name_value_map = features[i];
        for (auto it = name_value_map.begin(); it != name_value_map.end(); ++it) {
            auto &fname = it->first;
            auto iter = feature_map.find(fname);
            if (iter == feature_map.end()) {
                continue;
            }
            auto fid = iter->second;
            auto fvalue = it->second;
            data[i][fid] = fvalue;
        }
    }
    auto ret = XGDMatrixCreateFromMat(reinterpret_cast<float*>(data),
                                      static_cast<bst_ulong>(nrow),
                                      static_cast<bst_ulong>(ncolumn),
                                      missing_value, &out);
    EXPECT_GE_OR_RETURN_LOGGED(ret, 0, false, "xgboost DMatrix create error!");
    return true;
}

bool XGBoostMgr::predict(
        const vector<unordered_map<string, float>> &features,
        vector<float> &out) {
    if (!out.empty()) {
        out.clear();
    }
    EXPECT_FALSE_OR_RETURN(features.empty(), false);
    // trans to xgb DMatrix
    DMatrixHandle dmatrix;
    trans_to_dmatrix(features, dmatrix);

    // xgb predict
    bst_ulong out_len;
    const float* pred_out = nullptr;
    auto ret = XGBoosterPredict(xgb_handler, dmatrix, 0, 0, &out_len, &pred_out);
    EXPECT_GE_OR_RETURN_LOGGED(ret, 0, false, "xgboost predixt failed");
    EXPECT_EQ_OR_RETURN_LOGGED(out_len, features.size(), false,
                                "xgboost input and output items cnt is not equal");

    // trans output format
    auto len = static_cast<uint64_t>(out_len);
    out.resize(len);
    for (uint64_t i = 0; i < len; ++i) {
        out[i] = *(pred_out + i);
    }
    XGDMatrixFree(dmatrix);
    return true;
}

};
