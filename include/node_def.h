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

using TermFreqMap = std::unordered_map<std::size_t, uint16_t>;

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
    uint16_t offset; // offset in unicode
    uint16_t length; // length in unicode, 即真实的字符长度, 一个汉字长度为1
    uint16_t dup; // 表示term第几次出现, 从1开始计数
    float wei; // tf * idf
};

// 正排 doc 节点
class DocInfo {
public:
    DocInfo() {}
    DocInfo(std::size_t sign,
            const std::string &t,
            const std::string &u,
            const std::vector<TermNode> &tms,
            const TermFreqMap &tf_map,
            float module = 0.0);
public:
    std::size_t doc_sign; // doc签名
    std::string title; // title明文
    uint32_t title_len; // unicode length
    std::string url; // url明文
    std::vector<TermNode> terms; // title切词
    TermFreqMap term_freq_map; // title中每个term的频次
    float vec_module; // terms tf*idf 向量的模
};

// 倒排 doc 节点; 可扩展
class DocNode {
public:
    DocNode() {}
    DocNode(std::size_t sign);
    bool operator==(const DocNode &other) const;
public:
    std::size_t doc_sign;
};

struct DocNodeHash {
    std::size_t operator()(const DocNode &node) const {
        return node.doc_sign;
    }
};

// 倒排 term 节点
class TermInfo {
public:
    TermInfo() {}
    TermInfo(std::size_t sign,
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
    std::vector<std::vector<std::string>> syns; // 每个term的同义词改写, 与term切词对齐
	TermFreqMap term_freq_map; // 每个term的出现频次: <sign, freq>
    float vec_module; // terms tf*idf 向量的模
};

// QU相关feature
class ResInfo {
public:
    ResInfo() {}
    ResInfo(const std::size_t &sign,
            std::shared_ptr<DocInfo> doc,
            const TermFreqMap &hit_terms);
    void update_res_info();
    bool operator < (const ResInfo &rhs) const;

public:
    std::size_t doc_sign; // doc签名
    std::shared_ptr<DocInfo> doc_info; // doc正排信息
    TermFreqMap hit_term_map; // doc与query的term交集: <sign, term在query,doc中的最小频次>
    TermFreqMap hit_syn_map; // query term同义词命中doc的情况: <sign, 同义词term在query中的index>
    std::shared_ptr<FeatureMgr> feature_mgr;

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
