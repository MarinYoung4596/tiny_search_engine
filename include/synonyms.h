/*****************************************************************
*   Copyright (C) 2019 . All rights reserved.
*   
*   @file:    synonyms.h
*   @author:  marinyoung@163.com
*   @date:    2019/06/27 00:49:38
*   @brief  
*
*****************************************************************/

#ifndef __SYNONYMS_H_
#define __SYNONYMS_H_

#include "common.h"
#include "config_util.h"
#include "log_util.h"
#include "str_util.h"
#include "node_def.h"
#include <unordered_map>
#include <string>

namespace tiny_engine {
using std::string;
using std::vector;
using std::unordered_map;

class Synonyms {
public:
    Synonyms() {}
    bool init(std::shared_ptr<ConfigUtil> configs);
    bool get_syns(const string &term, vector<string> &out) const;
    bool get_syns(const string &term, vector<SynTermNode> &out) const;
    float similarity(const string &first, const string &second);

private:
    DISALLOW_COPY_AND_ASSIGN(Synonyms);

    bool _load_data(const string &fpath);

private:
    unordered_map<string, string> term_idx_dict; // term, cluster_id
    unordered_map<string, vector<string>> idx_terms_dict; // cluster_id, terms
};

};

#endif
/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
