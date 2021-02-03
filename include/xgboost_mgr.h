/*****************************************************************
*   Copyright (C) 2021  Inc. All rights reserved.
*
*   @file:    xgboost_mgr.h
*   @author:  marinyoung@163.com
*   @date:    2021/01/28 23:31:20
*   @brief:   
*   @ref:     https://xgboost.readthedocs.io/en/latest/dev/c__api_8h.html
              https://stackoverflow.com/questions/36071672/using-xgboost-in-c
              https://github.com/EmbolismSoil/xgboostpp/blob/master/xgboostpp.h
              https://www.it1352.com/1984339.html
              https://github.com/R-Stalker/xgboost-c-cplusplus
              https://github.com/hawhuang/xgboost_realtime_predict_test
              https://github.com/lengender/xgboost_model
*
*****************************************************************/

#pragma once

#include "common.h"
#include "file_util.h"
#include "str_util.h"
#include "log_util.h"
#include "config_util.h"
#include "../third_party/xgboost/c_api.h"
#include <unordered_map>
#include <vector>
#include <string>
#include <memory> // shared_ptr

namespace tiny_engine {
using std::unordered_map;
using std::vector;
using std::string;

class XGBoostMgr {
public:
    XGBoostMgr(const string &model = string(),
                const string &feature = string(),
                float missing = 0.0);
    ~XGBoostMgr();
    bool init();
    bool init(std::shared_ptr<ConfigUtil> configs);
    bool load_feature_map(const string &fpath);
    bool load_model(const string &fpath);
    bool predict(
            const vector<unordered_map<string, float>> &features,
            vector<float> &out);
    bool trans_to_dmatrix(
            const vector<unordered_map<string, float>> &features,
            DMatrixHandle &out) const;
    void trans_to_svm_data(
            const unordered_map<string, float> &features,
            unordered_map<int, float> &out) const;

private:
    unordered_map<string, int> feature_map;
    BoosterHandle xgb_handler;

    float missing_value;
    string model_bin_path;
    string feature_map_path;
};
};
