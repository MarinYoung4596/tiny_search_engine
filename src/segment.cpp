/*****************************************************************
*   Copyright (C) 2018 . All rights reserved.
*
*   @file:    segment.cpp
*   @author:  marinyoung@163.com
*   @date:    2018/12/27 22:50:48
*   @brief  
*
*****************************************************************/


#include "../include/segment.h"

namespace tiny_engine {

TermNode::TermNode(std::size_t sign,
        const std::string &txt,
        uint32_t off,
        uint32_t len,
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

bool Segment::init(std::shared_ptr<ConfigUtil> configs) {
    EXPECT_NE_OR_RETURN(nullptr, configs, false);

    std::string jieba_dict_path;
    if (!configs->get_value("JIEBA_DICT", jieba_dict_path)) {
        jieba_dict_path = "./dict/jieba.dict.utf8";
        LOG_WARNING("key[JIEBA_DICT] not found, set to [%s]", jieba_dict_path.c_str());
    }

    std::string hmm_model_path;
    if (!configs->get_value("HMM_MODEL", hmm_model_path)) {
        hmm_model_path = "./dict/hmm_model.utf8";
        LOG_WARNING("key[HMM_MODEL] not found, set to [%s]", hmm_model_path.c_str());
    }

    std::string user_dict_path;
    if (!configs->get_value("USER_DICT", user_dict_path)) {
        user_dict_path = "./dict/user.dict.utf8";
        LOG_WARNING("key[USER_DICT] not found, set to [%s]", user_dict_path.c_str());
    }

    std::string idf_dict_path;
    if (!configs->get_value("IDF_DICT", idf_dict_path)) {
        idf_dict_path = "./dict/idf.utf8";
        LOG_WARNING("key[IDF_DICT] not found, set to [%s]", idf_dict_path.c_str());
    }

    std::string stop_words_path;
    if (!configs->get_value("STOP_WORDS", stop_words_path)) {
        stop_words_path = "./dict/stop_words.utf8";
        LOG_WARNING("key[STOP_WORDS] not found, set to [%s]", stop_words_path.c_str());
    }
    
    jieba = std::make_shared<cppjieba::Jieba>(
        jieba_dict_path,
        hmm_model_path,
        user_dict_path,
        idf_dict_path,
        stop_words_path
    );
    
    return nullptr != jieba;
}

bool Segment::get_token(const std::string &str,
        std::vector<TermNode> &tokens,
        std::unordered_map<std::size_t, uint16_t>* tf_map,
        TOKEN_TYPE type) {
    EXPECT_FALSE_OR_RETURN(str.empty(), false);
    if (!tokens.empty()) {
        tokens.clear();
    }
    std::vector<cppjieba::Word> seg_results;
    EXPECT_TRUE_OR_RETURN(_get_token(str, seg_results, type), false);
    std::unordered_map<std::size_t, uint16_t> term_freq_map;
    for (const auto &term : seg_results) {
        auto sign = StrUtil::str_to_sign(StrUtil::to_lower_case(term.word));
        auto iter = term_freq_map.find(sign);
        if (iter != term_freq_map.end()) {
            ++(iter->second);
        } else {
            term_freq_map.insert(std::make_pair(sign, 1));
        }
        TermNode node(sign,
                term.word,
                term.unicode_offset,
                term.unicode_length,
                term_freq_map[sign]);
        tokens.push_back(std::move(node));
    }
    if (nullptr != tf_map) {
        *tf_map = term_freq_map;
    }
    return true;
}

bool Segment::_get_token(const std::string &str,
        std::vector<cppjieba::Word> &seg_results,
        TOKEN_TYPE type) {
    switch (type) {
        case MP_MODE:
            jieba->Cut(str, seg_results, false);
            break;
        case HMM_MODE:
            jieba->CutHMM(str, seg_results);
            break;
        case SEARCH_MODE:
            jieba->CutForSearch(str, seg_results, true);
            break;
        case ALL_MODE:
            jieba->CutAll(str, seg_results);
            break;
        case MIX_MODE:
            jieba->Cut(str, seg_results, true);
            break;
        default:
            return false;
    }
    return true;
}

}; // end of namespace tiny_engine

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
