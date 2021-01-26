/*****************************************************************
*   Copyright (C) 2019 . All rights reserved.
*   
*   @file:    synonyms.cpp
*   @author:  marinyoung@163.com
*   @date:    2019/06/27 00:50:21
*   @brief  
*
*****************************************************************/

#include "../include/synonyms.h"

namespace tiny_engine {

bool Synonyms::init(std::shared_ptr<ConfigUtil> configs) {
    EXPECT_NE_OR_RETURN(nullptr, configs, false);
    string fpath;
    if (!configs->get_value("SYN_DICT", fpath)) {
        fpath = "./dict/cilin_ex.txt";
    }
    return _load_data(fpath);
}

bool Synonyms::_load_data(const string &file) {
    EXPECT_FALSE_OR_RETURN(file.empty(), false);
    if (!FileUtil::is_readable(file)) {
        LOG_WARNING("file[%s] is not exist/readable", file.c_str());
        return false;
    }
    std::ifstream ifs(file.c_str(), std::ifstream::in);
    if (!ifs.is_open()) {
        LOG_WARNING("open file[%s] failed", file.c_str());
        return false;
    }
    std::string line;
    vector<string> vec;
    while (!ifs.eof()) {
        std::getline(ifs, line);
        StrUtil::split(line, ' ', vec);
        if (vec.size() < 2) {
            continue;
        }
        auto &cluster_id = vec[0];
        vector<string> cluster(std::next(vec.begin()), vec.end());
        idx_terms_dict[cluster_id] = cluster;
        for (const auto &item : cluster) {
            term_idx_dict[item] = cluster_id;
        }
        vec.clear();
        line.clear();
    }
    LOG_INFO("load syn dict from %s ok, idx_dict=%lu term_dict=%lu",
            file.c_str(), idx_terms_dict.size(), term_idx_dict.size());
    return true;
}

bool Synonyms::get_syns(const string &term, vector<string> &result) {
    EXPECT_FALSE_OR_RETURN(term.empty(), false);
    if (!result.empty()) {
        result.clear();
    }
    auto iter1 = term_idx_dict.find(term);
    EXPECT_NE_OR_RETURN(iter1, term_idx_dict.end(), false);
    auto iter2 = idx_terms_dict.find(iter1->second);
    EXPECT_NE_OR_RETURN(iter2, idx_terms_dict.end(), false);
    for (const auto &item : iter2->second) {
        result.push_back(item);
    }
    return true;
}

};

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
