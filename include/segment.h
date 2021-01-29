/*****************************************************************
*   Copyright (C) 2018 . All rights reserved.
*   
*   @file:    segment.h
*   @author:  marinyoung@163.com
*   @date:    2018/12/11 11:30:06
*   @brief  
*
*****************************************************************/


#ifndef __SEGMENT_H_
#define __SEGMENT_H_

#include "common.h"
#include "str_util.h"
#include "log_util.h"
#include "config_util.h"
#include "../third_party/jieba/Jieba.hpp"
#include <memory>

namespace tiny_engine {
/**
 * ref:
 * https://github.com/yanyiwu/libcppjieba/blob/master/include/Application.hpp
 * https://github.com/yanyiwu/cppjieba/blob/master/test/demo.cpp
 */
enum TOKEN_TYPE {
    MP_MODE = 1,
    HMM_MODE = 2,
    ALL_MODE = 3,
    SEARCH_MODE = 4,
    MIX_MODE = 5
};

class TermNode {
public:
    TermNode(std::size_t sign,
            const std::string &txt,
            uint32_t off = 0,
            uint32_t len = 0,
            uint16_t dup = 0,
            float wei = 0.0);
    TermNode(const TermNode &other);
    TermNode& operator=(const TermNode &other);
    bool operator==(const TermNode &other) const;
    bool operator<(const TermNode &other) const;
    std::string to_str() const;

public:
    std::size_t token_sign; // term签名
    std::string token;  // term明文
    uint32_t offset; // offset in unicode
    uint32_t length; // length in unicode
    uint16_t dup; // 表示term第几次出现, 从1开始计数
    float wei; // tf * idf
};

class Segment {
public:
    Segment() {}

    bool init(std::shared_ptr<ConfigUtil> configs);

    bool get_token(const std::string &str,
            std::vector<TermNode> &tokens,
            std::unordered_map<std::size_t, uint16_t>* tf_map = nullptr,
            TOKEN_TYPE type = MIX_MODE);

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
