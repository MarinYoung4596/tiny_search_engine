/*****************************************************************
*   Copyright (C) 2021  Inc. All rights reserved.
*
*   @file:    node_def.cpp
*   @author:  marinyoung@163.com
*   @date:    2021/02/03 23:23:03
*   @brief:   
*
*****************************************************************/


#include "../include/node_def.h"

namespace tiny_engine {

TermNode::TermNode(std::size_t sign,
        const std::string &txt,
        uint16_t off,
        uint16_t len,
        uint16_t dup,
        float wei) :
    token_sign(sign),
    token(txt),
    offset(off),
    length(len),
    dup(dup),
    wei(wei) {}

TermNode::TermNode(const TermNode &other) {
    if (this != &other) {
        token_sign = other.token_sign;
        token = other.token;
        offset = other.offset;
        length = other.length;
        dup = other.dup;
        wei = other.wei;
    }
}

TermNode& TermNode::operator=(const TermNode &other) {
    if (this != &other) {
        token_sign = other.token_sign;
        token = other.token;
        offset = other.offset;
        length = other.length;
        dup = other.dup;
        wei = other.wei;
    }
    return *this;
}

bool TermNode::operator==(const TermNode &other) const {
    return token_sign == other.token_sign;
}

bool TermNode::operator<(const TermNode &other) const {
    return token < other.token;
}

std::string TermNode::to_str() const {
#ifdef DEBUG
    return StrUtil::format(
        "{},off={},len={},dup={},wei={},sign={}",
        token.c_str(), offset, length, dup, wei, token_sign);
#else
    return token;
#endif
}

// ----------------------------------------------------------------

DocInfo::DocInfo(
        std::size_t sign,
        const std::string &t,
        const std::string &u,
        const std::vector<TermNode> &tms,
        const TermFreqMap &tf_map,
        float module_) :
    doc_sign(sign),
    title(t),
    url(u),
    terms(tms),
    term_freq_map(tf_map),
    vec_module(module_) {
        if (!terms.empty()) {
            auto p_last_term = std::prev(terms.end());
            title_len = p_last_term->offset + p_last_term->length;
        }
    }

// -----------------------------------------------------------

DocNode::DocNode(std::size_t sign) : doc_sign(sign) {}

bool DocNode::operator==(const DocNode &other) const {
    return doc_sign == other.doc_sign;
}

// -----------------------------------------------------------

TermInfo::TermInfo(
        std::size_t sign,
        const std::string &t,
        uint16_t len,
        uint16_t tf,
        float idf) :
    term_sign(sign),
    term_txt(t),
    term_len(len),
    term_freq(tf),
    idf(idf) {}

// -----------------------------------------------------------

QueryInfo::QueryInfo(const std::string &q) : query(q) {}

void QueryInfo::init() {
    query.clear();
    terms.clear();
    syns.clear();
    vec_module = 0.0;
}

std::string QueryInfo::to_str() const {
    std::vector<std::string> vec;
    for (auto it = terms.begin(); it != terms.end(); ++it) {
        vec.push_back(it->to_str());
    }
    return StrUtil::join(vec.begin(), vec.end(), '\t');
}

// -----------------------------------------------------------

ResInfo::ResInfo(
        const std::size_t &sign,
        std::shared_ptr<DocInfo> doc,
        const TermFreqMap &hit_terms) :
    doc_sign(sign),
    doc_info(doc),
    hit_term_map(hit_terms),
    term_hits(0),
    vsm(0.0),
    cqr(0.0),
    ctr(0.0),
    bm25(0.0),
    miss(0.0),
    extra(0.0),
    disorder(0.0),
    final_score(0.0) {
    update_res_info();
    if (nullptr == feature_mgr) {
        feature_mgr = std::make_shared<FeatureMgr>();
    }
}

void ResInfo::update_res_info() {
    uint16_t sum = 0;
    term_hits = std::accumulate(
        hit_term_map.begin(), hit_term_map.end(), sum,
        [&](uint16_t sum, const std::pair<std::size_t, uint16_t> &item) -> uint16_t {
            return sum + item.second;
        }
    );
}

};
