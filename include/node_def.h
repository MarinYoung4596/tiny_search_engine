/*****************************************************************
*   Copyright (C) 2021  Inc. All rights reserved.
*
*   @file:    node_def.h
*   @author:  marinyoung@163.com
*   @date:    2021/02/03 23:22:53
*   @brief:   
*
*****************************************************************/


#pragma once
#include "str_util.h"
#include "feature_mgr.h"

#include <memory>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace tiny_engine {

// 切词信息
class TermNode {
public:
    TermNode(std::size_t sign,
            const std::string &txt,
            uint16_t off = 0,
            uint16_t len = 0,
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
    uint16_t offset; // offset in unicode, 字符粒度
    uint16_t length; // length in unicode, 即真实的字符长度, 一个汉字长度为1
    uint16_t dup; // 表示term第几次出现, 从1开始计数
    float wei; // tf * idf
};

// term 同义词改写节点
class SynTermNode {
public:
    SynTermNode(const std::string &txt, float belief = 0.0);
public:
    std::size_t token_sign;
    std::string token;
    float belief; // 与原词的改写置信度
};

// term 在原始字符串片段中的信息
class GlobalTermInfo {
public:
    GlobalTermInfo(std::size_t pts = 0,
                   uint16_t tf = 0,
                   uint16_t ti = 0);
public:
    std::size_t pre_term_sign;
    uint16_t term_freq;
    uint16_t term_idx; // 首次出现的 term_index
    std::vector<uint16_t> offsets; // 所有字符粒度的 offset
};

// 正排 value: 正排 doc 节点, doc相关feature
class DocInfo {
public:
    DocInfo() {}
    DocInfo(std::size_t sign,
            const std::string &t,
            const std::string &u,
            const std::vector<TermNode> &tms,
            float module = 0.0);
public:
    std::size_t doc_sign; // doc签名
    std::string title; // title明文
    std::string url; // url明文
    uint32_t title_len; // unicode length
    std::vector<TermNode> terms; // title切词
    std::unordered_map<std::size_t, GlobalTermInfo> term_map;
    float vec_module; // 所有切词terms tf*idf 向量的模
    float wei_sum; // ∑ idf
};

// 倒排 doc 节点: 倒排的 value
class DocNode {
public:
    DocNode() {}
    DocNode(std::size_t sign);
    bool operator==(const DocNode &other) const;
    std::size_t operator()() const;
public:
    std::size_t doc_sign;
};

struct DocNodeHash {
    std::size_t operator()(const DocNode &node) const {
        return node.doc_sign;
    }
};

// 倒排 term 节点: 倒排的 key
class InvTermInfo {
public:
    InvTermInfo() {}
    InvTermInfo(std::size_t sign,
            const std::string &t,
            uint16_t len = 0,
            uint16_t tf = 0,
            float idf = 0.0);
public:
    std::size_t term_sign; // term签名
    std::string term_txt; // term明文
    uint16_t term_len; // term长度
    uint16_t term_freq; // term在所有doc中出现的总次数
    float idf;
    std::unordered_set<DocNode, DocNodeHash> docs;
};

// QU匹配的term信息
class MatchTermInfo {
public:
    MatchTermInfo();
    std::string to_str() const;

public:
    GlobalTermInfo in_query;
    GlobalTermInfo in_doc;

    std::size_t term_sign;
    std::string term_txt;
    uint16_t term_len;
    uint16_t hit_freq; // min(tf_in_query, tf_in_doc)
    float idf;
    bool is_syn_match;
    float syn_belief; // syn_confidence
    bool is_stopword;
};

// query相关feature
class QueryInfo {
public:
    QueryInfo() {}
    QueryInfo(const std::string &q);
    void init();
    std::string to_str() const;
public:
    std::string query; // query明文
    uint32_t query_len; // unicode length
    std::vector<TermNode> terms; // query切词
    std::vector<std::vector<SynTermNode>> syns; // 每个term的同义词改写, 与term切词对齐
    std::unordered_map<std::size_t, GlobalTermInfo> term_map; // key=sign
    float vec_module; // terms tf*idf 向量的模
    float wei_sum; // ∑ idf
};

// QU相关feature
class ResInfo {
public:
    ResInfo(const std::size_t dsign = 0,
            std::shared_ptr<DocInfo> doc = nullptr);
    void update_res_info();
    bool operator < (const ResInfo &rhs) const;

public:
    std::size_t doc_sign; // doc签名
    std::shared_ptr<DocInfo> doc_info; // doc正排信息
    std::unordered_map<uint64_t, std::shared_ptr<MatchTermInfo>> match_term_map;
    std::shared_ptr<FeatureMgr> feature_mgr;

    bool recall_by_syn; // 是否由同义词召回
    // qu相关性打分的特征
    uint16_t term_hits; // doc与query的term交集个数(包含重复)
    float vsm;
    float cqr;
    float ctr;
    float bm25;
    float miss; // 未命中term权重比例
    float extra; // 未命中term长度占比
    float disorder; // 逆序pair对的比例

    // rank score
    float final_score;
};

};
