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
        TOKEN_TYPE type) {
    EXPECT_FALSE_OR_RETURN(str.empty(), false);
    EXPECT_TRUE_OR_DO(tokens.empty(), tokens.clear());
    std::vector<cppjieba::Word> seg_results;
    EXPECT_TRUE_OR_RETURN(_get_token(str, seg_results, type), false);
    for (const auto &term : seg_results) {
        auto sign = StrUtil::str_to_sign(StrUtil::to_lower_case(term.word));
        TermNode node(sign,
                term.word,
                term.unicode_offset,
                term.unicode_length);
        tokens.push_back(std::move(node));
    }
    return true;
}

bool Segment::update_global_info(
            std::vector<TermNode> &tokens,
            std::unordered_map<std::size_t, GlobalTermInfo> &out) const {
    EXPECT_FALSE_OR_RETURN(tokens.empty(), false);
    EXPECT_TRUE_OR_DO(out.empty(), out.clear());

    std::size_t pre_term_sign = 0;
    for (auto i = 0; i < tokens.size(); ++i) {
        auto &token = tokens[i];
        auto tf = out.count(token.token_sign);
        token.dup = tf + 1;

        if (tf > 0) {
            out[token.token_sign].term_freq += 1;
            out[token.token_sign].offsets.push_back(token.offset);
        } else {
            GlobalTermInfo gti(pre_term_sign, 1, i);
            gti.offsets.push_back(token.offset);
            out[token.token_sign] = std::move(gti);
        }
        pre_term_sign = token.token_sign;
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
