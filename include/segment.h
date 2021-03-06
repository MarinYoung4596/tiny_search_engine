/*****************************************************************
*   Copyright (C) 2018 . All rights reserved.
*
*   @file:    segment.h
*   @author:  marinyoung@163.com
*   @date:    2018/12/11 11:30:06
*   @brief:
*   @ref:     https://github.com/yanyiwu/libcppjieba/blob/master/include/Application.hpp
              https://github.com/yanyiwu/cppjieba/blob/master/test/demo.cpp
*
*****************************************************************/


#ifndef __SEGMENT_H_
#define __SEGMENT_H_

#include "common.h"
#include "str_util.h"
#include "log_util.h"
#include "config_util.h"
#include "node_def.h"
#include "../third_party/jieba/Jieba.hpp"
#include <memory>

namespace tiny_engine {

enum TOKEN_TYPE {
    MP_MODE = 1,
    HMM_MODE = 2,
    ALL_MODE = 3,
    SEARCH_MODE = 4,
    MIX_MODE = 5
};

class Segment {
public:
    Segment() {}

    bool init(std::shared_ptr<ConfigUtil> configs);

    bool get_token(const std::string &str,
            std::vector<TermNode> &tokens,
            TOKEN_TYPE type = MIX_MODE);

    bool update_global_info(
            std::vector<TermNode> &tokens,
            std::unordered_map<std::size_t, GlobalTermInfo> &out) const;

private:
    DISALLOW_COPY_AND_ASSIGN(Segment);

    bool _get_token(const std::string &str,
            std::vector<cppjieba::Word> &seg_results,
            TOKEN_TYPE type = MIX_MODE);

private:
    std::shared_ptr<cppjieba::Jieba> jieba;
};

}; // end of namespace tiny_engine
#endif // __SEGMENT_H_

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
