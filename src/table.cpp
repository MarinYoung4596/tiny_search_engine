/*****************************************************************
*   Copyright (C) 2018 . All rights reserved.
*
*   @file:    table.cpp
*   @author:  marinyoung@163.com
*   @date:    2018/11/21 22:53:57
*   @brief:   
*
*****************************************************************/

#include "../include/table.h"
#include <omp.h>

namespace tiny_engine {

Table::Table() {
    forward_file_path = new char[MAX_FILE_PATH_LEN::value];
    invert_file_path = new char[MAX_FILE_PATH_LEN::value];
    index_file_path = new char[MAX_FILE_PATH_LEN::value];
}

Table::~Table() {
    SAFE_DELETE_ARR_PTR(forward_file_path);
    SAFE_DELETE_ARR_PTR(invert_file_path);
    SAFE_DELETE_ARR_PTR(index_file_path);
}

bool Table::init(std::shared_ptr<ConfigUtil> configs,
        std::shared_ptr<Segment> wordseg) {
    EXPECT_NE_OR_RETURN(nullptr, configs, false);
    std::string value;
    if (!configs->get_value("FORWARD_TABLE", value)) {
        value = "./index/fwd.list";
        LOG_WARNING("key[FORWARD_TABLE] not found, set to[%s]", value.c_str());
    }
    set_fwd_path(value);

    if (!configs->get_value("INVERTED_TABLE", value)) {
        value = "./index/inv.list";
        LOG_WARNING("key[INVERTED_TABLE] not found, set to[%s]", value.c_str());
    }
    set_inv_path(value);

    if (!configs->get_value("INDEX_FILE", value)) {
        value = "./index/url.list.utf8";
        LOG_WARNING("key[INDEX_FILE] not found, set to[%s]", value.c_str());
    }
    set_index_path(value);

    if (!configs->get_value("STOPWORD", value)) {
        value = "./dict/stopword.utf8";
        LOG_WARNING("key[STOPWORD] not found, set to[%s]", value.c_str());
    }
    if (!_load_stopword(value)) {
        LOG_WARNING("load stopword[%s] failed", value.c_str());
    }

    int index_load_option = 0;
    if (!configs->get_value("LOAD_FROM_JSON", index_load_option)) {
        index_load_option = 0;
        LOG_WARNING("key[LOAD_FROM_JSON] not found, set to[%d]", index_load_option);
    }
    if (1 == index_load_option) {
        EXPECT_TRUE_OR_RETURN_LOGGED(load_index_from_json(), false, "load index error");
    } else if (nullptr != wordseg) {
        EXPECT_TRUE_OR_RETURN_LOGGED(load_index_from_file(wordseg), false, "load index error");
#ifdef DEBUG
        auto time_begin = TimeUtil::get_curr_timeval();
#endif
        EXPECT_TRUE_OR_RETURN(_update_inv_table(), false);
        EXPECT_TRUE_OR_RETURN(_update_fwd_table(), false);
#ifdef DEBUG
        auto time_end = TimeUtil::get_curr_timeval();
        auto delta_time = TimeUtil::timeval_diff_ms(&time_end, &time_begin);
        LOG_DEBUG("index update ok, cost %lu ms", delta_time);
#endif
    }
    configs->get_value("MAX_SYN_TERM_RECALL", max_syn_term_recall);
    return true;
}

inline void Table::set_fwd_path(const std::string &str) {
    strncpy(forward_file_path, str.c_str(), MAX_FILE_PATH_LEN::value);
}

inline void Table::set_inv_path(const std::string &str) {
    strncpy(invert_file_path, str.c_str(), MAX_FILE_PATH_LEN::value);
}

inline void Table::set_index_path(const std::string &str) {
    strncpy(index_file_path, str.c_str(), MAX_FILE_PATH_LEN::value);
}

inline std::size_t Table::get_fwd_size() const {
    return forward_table.size();
}

inline std::size_t Table::get_inv_size() const {
    return invert_table.size();
}

inline std::shared_ptr<DocInfo> Table::get_doc_info(const std::size_t &doc_sign) const {
    auto iter = forward_table.find(doc_sign);
    return iter != forward_table.end() ? iter->second : nullptr;
}

inline std::shared_ptr<InvTermInfo> Table::get_term_info(const std::size_t &term_sign) const {
    auto iter = invert_table.find(term_sign);
    return iter != invert_table.end() ? iter->second : nullptr;
}

inline float Table::get_term_idf(const std::size_t &term_sign) const {
    auto term_info = get_term_info(term_sign);
    return nullptr != term_info ? term_info->idf : 0.0;
}

inline std::string Table::get_term_txt(const std::size_t &term_sign) const {
    auto term_info = get_term_info(term_sign);
    return nullptr != term_info ? term_info->term_txt : std::string();
}

bool Table::_load_stopword(const std::string &file) {
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
    while (!ifs.eof()) {
        std::getline(ifs, line);
        line = StrUtil::right_trim_with(line, '\n');
        stopword.insert(StrUtil::str_to_sign(line));
    }
    LOG_INFO("load %lu items from %s", stopword.size(), file.c_str());
    return true;
}

inline bool Table::hit_stopword(const std::size_t &term_sign) const {
    return stopword.find(term_sign) != stopword.end();
}

bool Table::load_index_from_file(std::shared_ptr<Segment> wordseg) {
    EXPECT_TRUE_OR_RETURN_LOGGED(
            FileUtil::is_readable(index_file_path),
            false, "file not exist");
    auto time_begin = TimeUtil::get_curr_timeval();
    std::ifstream ifs(index_file_path, std::ios::in);
    EXPECT_TRUE_OR_RETURN_LOGGED(ifs.is_open(), false, "open file error");

    std::string line;
    std::vector<std::string> vec;
    while (!ifs.eof()) {
        std::getline(ifs, line);
        StrUtil::split(line, '\t', vec);
        if (vec.size() < 3) {
            continue;
        }
        // query \t title \t url \t label
        auto &title = vec[1];
        auto &url = vec[2];
        std::vector<TermNode> tokens;
        if (!(wordseg->get_token(title, tokens))) {
            LOG_WARNING("segment title[%s] failed", title.c_str());
            continue;
        }
        auto doc_sign = StrUtil::str_to_sign(url); /////
        std::shared_ptr<DocInfo> doc_info(
                new DocInfo(doc_sign, title, url, tokens));
        wordseg->update_global_info(tokens, doc_info->term_map);
        if (!add_index(doc_info)) {
            LOG_WARNING("index build error, query[%s] title[%s]",
                    vec[0].c_str(), title.c_str());
        }
    }
    ifs.close();
    calc_avg_doc_len();
    auto time_end = TimeUtil::get_curr_timeval();
    auto delta_time = TimeUtil::timeval_diff_ms(&time_end, &time_begin);
    LOG_INFO("initialize index from %s ok, fwd=%lu inv=%lu cost=%lu ms",
            index_file_path, get_fwd_size(), get_inv_size(), delta_time);
    return true;
}

inline bool Table::load_index_from_json() {
    return _load_fwd_table() && _load_inv_table();
}

inline bool Table::dump_index() const {
    return _dump_fwd_table() && _dump_inv_table();
}

inline bool Table::add_index(std::shared_ptr<DocInfo> doc_info) {
    EXPECT_NE_OR_RETURN(nullptr, doc_info, false);
    return _add_to_inv_table(doc_info) && _add_to_fwd_table(doc_info);
}

bool Table::_load_fwd_table() {
    // TODO: load forward table from json str
    return true;
}

bool Table::_load_inv_table() {
    // TODO: load inverted table from json str
    return true;
}

bool Table::_dump_fwd_table() const {
    // TODO: dump forward table into json str
#ifdef DEBUG
    std::ofstream ofs(forward_file_path, std::ofstream::out);
    EXPECT_TRUE_OR_RETURN(ofs.is_open(), false);
    for (auto it = forward_table.begin(); it != forward_table.end(); ++it) {
        auto doc_info = it->second;
        ofs << doc_info->doc_sign \
            << '\t' << doc_info->title \
            << '\n';
    }
    ofs.close();
    EXPECT_TRUE_OR_RETURN_LOGGED(
            FileUtil::generate_md5_file(forward_file_path),
            false, "generate md5 file failed");
#endif
    return true;
}

bool Table::_dump_inv_table() const {
    // TODO: dump inverted table into json str
#ifdef DEBUG
    std::ofstream ofs(invert_file_path, std::ofstream::out);
    EXPECT_TRUE_OR_RETURN(ofs.is_open(), false);
    for (auto it = invert_table.begin(); it != invert_table.end(); ++it) {
        auto term_info = it->second;
        ofs << term_info->term_txt \
            << '\t' << term_info->idf \
            << '\t' << term_info->docs.size() \
            << '\n';
    }
    ofs.close();
    EXPECT_TRUE_OR_RETURN_LOGGED(
            FileUtil::generate_md5_file(invert_file_path),
            false, "generate md5 file failed");

#endif
    return true;
}

bool Table::_add_to_fwd_table(std::shared_ptr<DocInfo> doc_info, bool update_wei) {
    EXPECT_NE_OR_RETURN(nullptr, doc_info, false);
    if (update_wei) {
        for (auto &term : doc_info->terms) {
            if (hit_stopword(term.token_sign)) {
                continue;
            }
            auto idf = get_term_idf(term.token_sign);
            auto tf = term.dup;
            term.wei = idf * sqrt(tf) / sqrt(doc_info->terms.size());
        }
    }
    forward_table[doc_info->doc_sign] = doc_info;
    return true;
}

bool Table::_add_to_inv_table(std::shared_ptr<DocInfo> doc_info, bool update_idf) {
    EXPECT_NE_OR_RETURN(nullptr, doc_info, false);
    EXPECT_FALSE_OR_RETURN(doc_info->terms.empty(), false);
    for (auto it = doc_info->terms.crbegin(); it != doc_info->terms.crend(); ++it) {
        auto &term_sign = it->token_sign;
        if (hit_stopword(term_sign)) {
            continue;
        }
        bool need_update = false;
        std::shared_ptr<InvTermInfo> term_info = nullptr;
        if (invert_table.find(term_sign) != invert_table.end()) {
            term_info = invert_table[term_sign];
        } else {
            term_info = std::make_shared<InvTermInfo>(it->token_sign, it->token, it->length);
            need_update = true;
        }
        DocNode node(doc_info->doc_sign);
        // 从后往前, 同一个doc只插入一次, 保证doc中term频次为最大值
        if (term_info->docs.find(node) == term_info->docs.end()) {
            term_info->docs.insert(std::move(node));
            term_info->term_freq += it->dup;
            need_update = true;
        }
        if (need_update) {
            invert_table[term_sign] = term_info;
        }
        if (update_idf) {
            term_info->idf = MathUtil::calculate_idf(term_info->term_freq, get_fwd_size());
        }
    }
    return true;
}

bool Table::_update_inv_table() {
    for (auto inv_iter = invert_table.begin(); inv_iter != invert_table.end(); ++inv_iter) {
        auto &term_info = inv_iter->second;
        term_info->idf = MathUtil::calculate_idf(term_info->term_freq, get_fwd_size());
    }
    return true;
}

bool Table::_update_fwd_table() {
    for (auto &item : forward_table) {
        auto module_ = 0.0;
        auto wei_sum = 0.0;
        for (auto &term : item.second->terms) {
            if (hit_stopword(term.token_sign)) {
                continue;
            }
            auto idf = get_term_idf(term.token_sign);
            auto tf = term.dup;
            term.wei = idf; // * tf;
            module_ += pow(term.wei, 2.0);
            wei_sum += term.wei;
        }
        item.second->vec_module = sqrt(module_);
        item.second->wei_sum = wei_sum;
    }
    return true;
}

void Table::calc_avg_doc_len() {
    auto doc_num = get_fwd_size();
    EXPECT_GT_OR_RETURN(doc_num, 0, RETURN_ON_VOID);
    uint64_t sum_doc_len = 0;
    for (const auto &item : forward_table) {
        sum_doc_len += item.second->title_len;
    }
    avg_doc_len = static_cast<float>(sum_doc_len) / doc_num;
}

inline float Table::get_avg_doc_len() const {
    return avg_doc_len;
}

bool Table::recall(std::shared_ptr<QueryInfo> query_info,
        std::vector<std::shared_ptr<ResInfo>> &result) const {
    EXPECT_TRUE_OR_DO(result.empty(), result.clear());
    std::unordered_map<std::size_t, std::shared_ptr<ResInfo>> res_map;
    for (auto i = 0; i < query_info->terms.size(); ++i) { // each term
        auto &term = query_info->terms[i]; // TermNode
        if (term.dup > 1) { // 重复 term 不必召回
            continue;
        }
        auto term_info = get_term_info(term.token_sign); // InvTermInfo
        if (CHECK_NULL(term_info)) {
            continue;
        }
        // 处理原词召回情况
        auto doc_list = term_info->docs;
        LOG_DEBUG("term[%s] recall %lu docs", term_info->term_txt.c_str(), doc_list.size());
        for (auto doc : doc_list) {
            auto iter = res_map.find(doc.doc_sign);
            if (iter != res_map.end()) { // 当前 doc 在 res_map 中
                if (iter->second->match_term_map.count(term.token_sign) > 0) {
                    continue; // 召回结果中已存在该term的信息，不更新
                }
                // 否则,更新result里的match_term_info信息
                auto doc_info = iter->second->doc_info;
                auto mti = std::make_shared<MatchTermInfo>();
                fill_match_term_info(term_info, query_info, doc_info, mti);
                iter->second->match_term_map.insert(std::make_pair(term.token_sign, mti));
            } else { // 否则，当前结果插入 res_map
                process_new_doc(query_info, term_info, doc.doc_sign, false, i, res_map);
            }
        }
        //LOG_DEBUG("doc process ok, start process syn recall");
        // 处理同义词召回情况
        auto &syn_term_list = query_info->syns[i];
        for (auto j = 0; j < syn_term_list.size() && j < max_syn_term_recall; ++j) {
            auto &syn_term = syn_term_list[j];
            if (syn_term.token_sign == term.token_sign) {
                continue; // 同义词与原词完全一致
            }
            auto syn_term_info = get_term_info(syn_term.token_sign);
            if (CHECK_NULL(syn_term_info)) {
                continue;
            }
            auto &syn_doc_list = syn_term_info->docs;
            LOG_DEBUG("syn_term[%s] recall %lu docs", syn_term.token.c_str(), syn_doc_list.size());
            for (auto doc : syn_doc_list) {
                auto iter = res_map.find(doc.doc_sign);
                if (iter != res_map.end()) {
                    // 原词命中的同时，同义词也命中了，直接忽略
                    continue;
                }
                // 处理未命中的情况
                process_new_doc(query_info, syn_term_info, doc.doc_sign, true, i, res_map);
            }
        } // end of syn term recall
    } // end of query term traversal
    for (auto it = res_map.begin(); it != res_map.end(); ++it) {
        it->second->update_res_info();
        it->second->feature_mgr->add_feature("F_QU_HIT_TERM_CNT", it->second->term_hits);
        result.push_back(it->second);
    }
    // 1st_sort: sort by term_hits
    std::sort(result.begin(), result.end(),
            [&](const std::shared_ptr<ResInfo> &lhs, const std::shared_ptr<ResInfo> &rhs) {
        if (lhs->term_hits != rhs->term_hits) {
            return lhs->term_hits > rhs->term_hits;
        } else if (lhs->recall_by_syn != rhs->recall_by_syn) {
            return lhs->recall_by_syn < rhs->recall_by_syn;
        } else {
            return lhs->doc_info->title_len < rhs->doc_info->title_len;
        }
    });
    return true;
}

bool Table::process_new_doc(
        std::shared_ptr<QueryInfo> query_info,
        std::shared_ptr<InvTermInfo> term_info,
        std::size_t doc_sign,
        uint16_t term_idx_in_q,
        bool is_syn_recall,
        std::unordered_map<std::size_t, std::shared_ptr<ResInfo>> &res_map) const {
    //LOG_DEBUG("process_new_doc start, doc_sign=%lu", doc_sign);
    auto res_node = std::make_shared<ResInfo>();
    res_node->doc_sign = doc_sign;
    res_node->doc_info = get_doc_info(doc_sign);
    res_node->recall_by_syn = is_syn_recall;
    auto mti = std::make_shared<MatchTermInfo>();
    if (is_syn_recall) {
        mti->is_syn_match = true;
        mti->in_query.term_idx = term_idx_in_q;
    }
    fill_match_term_info(term_info, query_info, res_node->doc_info, mti);
    // 以doc侧的term_sign为 key
    res_node->match_term_map.insert(std::make_pair(term_info->term_sign, mti));
    res_map.insert(std::make_pair(doc_sign, res_node));
    //LOG_DEBUG("process_new_doc done");
    return true;
}

bool Table::fill_match_term_info(
        std::shared_ptr<InvTermInfo> term_info,
        std::shared_ptr<QueryInfo> query_info,
        std::shared_ptr<DocInfo> doc_info,
        std::shared_ptr<MatchTermInfo> mti) const {
    //LOG_DEBUG("fill_match_term_info start, term=%s", term_info->term_txt.c_str());
    // 从倒排中取信息
    mti->term_sign = term_info->term_sign;
    mti->term_txt = term_info->term_txt;
    mti->term_len = term_info->term_len;
    mti->idf = term_info->idf;
    mti->is_stopword = hit_stopword(mti->term_sign);

    // 从 query 中取信息
    if (mti->is_syn_match) {
        auto ori_term_sign = query_info->terms[mti->in_query.term_idx].token_sign;
        mti->in_query = query_info->term_map[ori_term_sign]; // 同义词召回继承原词的特征（位置、频次等）
    } else { // 原词命中
        mti->in_query = query_info->term_map[mti->term_sign];
    }

    // 从 doc 中取信息
    mti->in_doc = doc_info->term_map[mti->term_sign];

    // 其他信息
    mti->hit_freq = std::min(mti->in_query.term_freq, mti->in_doc.term_freq);
    //LOG_DEBUG("fill_match_term_info done");
    return true;
}

// -----------------------------------------------------------

TinyEngine::TinyEngine(
        const std::string &config,
        std::shared_ptr<Segment> _wordseg) :
    wordseg(_wordseg) {
    configs = std::make_shared<ConfigUtil>(config);
    query_info = std::make_shared<QueryInfo>();
    if (CHECK_NULL(wordseg)) {
        wordseg = std::make_shared<Segment>();
    }
    wordsyn = std::make_shared<Synonyms>();
    table.reset(new Table());
#ifdef XGBOOST
    xgb_mgr.reset(new XGBoostMgr());
#endif
}

bool TinyEngine::init() {
    // init config
    EXPECT_TRUE_OR_RETURN_LOGGED(configs->init(), false, "load config file failed");
    // init wordseg
    EXPECT_TRUE_OR_RETURN_LOGGED(wordseg->init(configs), false, "init wordseg error");
    // init wordsyn
    EXPECT_TRUE_OR_RETURN_LOGGED(wordsyn->init(configs), false, "init wordsyn error");
    // init index
    EXPECT_TRUE_OR_RETURN_LOGGED(table->init(configs, wordseg), false, "init index error");
#ifdef XGBOOST
    // load xgboost manager
    EXPECT_TRUE_OR_RETURN_LOGGED(xgb_mgr->init(configs), false, "init xgb mgr error");
#endif
    // load conf
    EXPECT_TRUE_OR_RETURN_LOGGED(_load_conf(), false, "load conf error");
    // load dict

    LOG_INFO("init search engine finished!");
    return true;
}

bool TinyEngine::stop() {
    // TODO
    EXPECT_TRUE_OR_RETURN_LOGGED(table->dump_index(), false, "dump index failed");
    return true;
}

bool TinyEngine::_load_conf() {
    if (!configs->get_value("MAX_INDEX_RECALL_NUM", _max_index_recall_num)) {
        _max_index_recall_num = MAX_INDEX_RECALL_NUM::value;
        LOG_WARNING("key[MAX_INDEX_RECALL_NUM] not found, set to [%d]", _max_index_recall_num);
    }
    if (!configs->get_value("MAX_2ND_SORT_NUM", _max_2nd_sort_num)) {
        _max_2nd_sort_num = MAX_SORT_NUM::value;
        LOG_WARNING("key[MAX_2ND_SORT_NUM] not found, set to [%d]", _max_2nd_sort_num);
    }
    if (!configs->get_value("MAX_RESULT_NUM", _max_result_num)) {
        _max_result_num = MAX_RESULT_NUM::value;
        LOG_WARNING("key[MAX_RESULT_NUM] not found, set to [%d]", _max_result_num);
    }
    int syn_highlight_flag;
    configs->get_value("HIGHLIGHT_SYN_MATCH_TERM", syn_highlight_flag);
    _is_highlight_syn_term = static_cast<bool>(syn_highlight_flag);
    return true;
}

bool TinyEngine::search(const std::string &query,
        std::vector<StrStrPair> &result) {
    EXPECT_FALSE_OR_RETURN(query.empty(), false);
    EXPECT_NE_OR_RETURN(nullptr, table, false);
    if (!result.empty()) {
        result.clear();
    }
    // 1. query segment
#ifdef DEBUG
    auto time_1st = TimeUtil::get_curr_timeval();
#endif
    EXPECT_TRUE_OR_RETURN(_fill_query_info(query), false);

    // 2. recall
#ifdef DEBUG
    auto time_2nd = TimeUtil::get_curr_timeval();
#endif
    if (!table->recall(query_info, results_info)) {
        LOG_WARNING("query[%s] recall failed", query.c_str());
    }

    // 3. ranking
#ifdef DEBUG
    auto time_3rd = TimeUtil::get_curr_timeval();
#endif
    _rank_results();

    // 4. truncation after final sort
#ifdef DEBUG
    auto time_4th = TimeUtil::get_curr_timeval();
#endif
    for (const auto &res : results_info) {
        if (result.size() >= _max_result_num) {
            break;
        }
        std::string title = _title_highlight(res);
#ifndef DEBUG
        std::cout << StrUtil::format("{}\t{}\t{}\n",
                                    query, res->doc_info->title, res->doc_info->url);
#endif

#ifdef DEBUG
        if (res->feature_mgr->is_empty()) {
            LOG_WARNING("query=%s title=%s feature is empty!",
                        query.c_str(), title.c_str());
        }
        std::cout << StrUtil::format(
                "{}\t{}\t{}\t{}\n",
                query,
                title,
                res->doc_info->url,
                res->feature_mgr->to_str()
        );
#endif
        result.push_back(StrStrPair(res->doc_info->title, res->doc_info->url));
    }

#ifdef DEBUG
    auto time_5th = TimeUtil::get_curr_timeval();
    auto time_segment = TimeUtil::timeval_diff_us(&time_2nd, &time_1st);
    auto time_recall = TimeUtil::timeval_diff_us(&time_3rd, &time_2nd);
    auto time_rank = TimeUtil::timeval_diff_us(&time_4th, &time_3rd);
    auto time_truncation = TimeUtil::timeval_diff_us(&time_5th, &time_4th);
    auto time_all = TimeUtil::timeval_diff_ms(&time_5th, &time_1st);
    LOG_INFO("query[%s] rn[%lu] tm{seg=%lu recall=%lu rank=%lu cut=%lu us; sum=%lu ms}",
            query.c_str(), results_info.size(),
            time_segment, time_recall, time_rank, time_truncation, time_all);
#endif
    return true;
}

bool TinyEngine::_fill_query_info(const std::string &query) {
    EXPECT_FALSE_OR_RETURN(query.empty(), false);
    EXPECT_NE_OR_RETURN(nullptr, wordseg, false);
    EXPECT_NE_OR_RETURN(nullptr, table, false);

    std::vector<TermNode> tokens;
    // word segment
    if (!(wordseg->get_token(query, tokens))) {
        LOG_WARNING("query[%s] segment error", query.c_str());
        return false;
    }
    // calculate tf * idf
    auto module_ = 0.0;
    auto wei_sum = 0.0;
    for (auto it = tokens.rbegin(); it != tokens.rend(); ++it) {
        if (table->hit_stopword(it->token_sign)) {
            continue;
        }
        auto idf = table->get_term_idf(it->token_sign);
        auto tf = it->dup;
        it->wei = idf; // 遍历，无需乘以 tf
        module_ += pow(it->wei, 2.0);
        wei_sum += it->wei;
    }
    // fill query info
    query_info->init();
    query_info->query = query;
    query_info->terms = std::move(tokens);
    query_info->vec_module = sqrt(module_);
    query_info->wei_sum = wei_sum;
    if (query_info->terms.size() > 0) {
        auto p_last_term = std::prev(query_info->terms.end());
        query_info->query_len = p_last_term->offset + p_last_term->length; // 真实字符个数
        wordseg->update_global_info(query_info->terms, query_info->term_map);
    }
    // get term syn info
    std::vector<SynTermNode> syns;
    for (const auto &term : query_info->terms) {
        wordsyn->get_syns(term.token, syns);
        query_info->syns.push_back(syns);
        syns.clear();
    }
    
    assert(query_info->terms.size() == query_info->syns.size());
#ifdef DEBUG
    std::vector<std::string> vec;
    for_each(query_info->terms.begin(), query_info->terms.end(),
        [&vec](const TermNode &node) {
            vec.push_back(node.to_str());
        });
    LOG_INFO("successfully filled query info, term_info[%s]",
        StrUtil::join(vec.begin(), vec.end(), " | ").c_str());
#endif
    return true;
}

bool TinyEngine::_rank_results() {
    #pragma omp parallel for num_threads(8)
    for (auto i = 0; i < results_info.size() && i < _max_2nd_sort_num; ++i) {
        _calc_features(results_info[i]);
    }
#ifdef XGBOOST
    std::vector<std::unordered_map<std::string, float>> all_doc_features;
    for (auto i = 0; i < results_info.size() && i < _max_2nd_sort_num; ++i) {
        auto res = results_info[i];
        all_doc_features.push_back(res->feature_mgr->get_features());
    }
    std::vector<float> predicts;
    auto ret = xgb_mgr->predict(all_doc_features, predicts);
    if (ret) {
        assert(predicts.size() == all_doc_features.size());
        for (decltype(results_info.size()) i = 0;
                i < results_info.size() && i < _max_2nd_sort_num; ++i) {
            auto res = results_info[i];
            res->final_score = predicts[i];
        }
    }
#endif
    std::sort(results_info.begin(), results_info.end(),
        [&](const std::shared_ptr<ResInfo> &lhs, const std::shared_ptr<ResInfo> &rhs) {
            return lhs->final_score > rhs->final_score;
        });
    return true;
}

bool TinyEngine::_calc_features(std::shared_ptr<ResInfo> result) {
    EXPECT_NE_OR_RETURN(nullptr, result, false);
    EXPECT_FALSE_OR_RETURN_LOGGED(result->match_term_map.empty(), false,
                                  "match_term_map is empty!");

    _calc_vsm(result);
    _calc_bm25(result);
    _calc_cqr_ctr(result);
    _calc_scatter_overlap(result);
    _calc_order_overlap(result);
    _calc_distance(result);
    _calc_disorder(result);
    _calc_diversity(result);
 
    result->feature_mgr->add_feature("F_QU_PROXIMITY",
                pow(0.9, result->miss + result->extra + result->disorder));
    result->final_score = result->vsm; //result->cqr * result->ctr;
    return true;
}

bool TinyEngine::_calc_vsm(std::shared_ptr<ResInfo> result) {
    auto &req_vec_module = query_info->vec_module;
    auto &res_vec_module = result->doc_info->vec_module;
    EXPECT_GT_OR_RETURN(abs(req_vec_module - 0.0), EPSILON, false);
    EXPECT_GT_OR_RETURN(abs(res_vec_module - 0.0), EPSILON, false);

    // sign : <req, res>
    std::unordered_map<std::size_t, std::pair<float, float>> term_wei_map;

    auto &req_terms = query_info->terms;
    for (const auto &term : req_terms) {
        if (table->hit_stopword(term.token_sign)) {
            continue;
        }
        term_wei_map.insert(std::make_pair(term.token_sign,
                                           std::make_pair(term.wei, 0.0)));
    }
    auto &res_terms = result->doc_info->terms;
    for (const auto &term : res_terms) {
        if (table->hit_stopword(term.token_sign)) {
            continue;
        }
        if (term_wei_map.count(term.token_sign)) {
            term_wei_map[term.token_sign].second = term.wei;
        } else {
            term_wei_map.insert(std::make_pair(term.token_sign,
                                               std::make_pair(0.0, term.wei)));
        }
    }

    std::vector<float> req_term_vec;
    std::vector<float> res_term_vec;
    for (auto it = term_wei_map.begin(); it != term_wei_map.end(); ++it) {
        req_term_vec.push_back(it->second.first);
        res_term_vec.push_back(it->second.second);
    }
    result->vsm = MathUtil::dot_product(req_term_vec, res_term_vec) / \
                  (req_vec_module * res_vec_module);
    
    result->feature_mgr->add_feature("F_QU_VSM", result->vsm);
    return true;
}

bool TinyEngine::_calc_bm25(std::shared_ptr<ResInfo> result) {
    auto avg_doc_len = table->get_avg_doc_len();
    float bm25 = 0.0;
    for (const auto &item : result->match_term_map) { // <term_sign, MatchTermInfo>
        auto wi = table->get_term_idf(item.first);

        auto term_freq_in_query = item.second->in_query.term_freq;
        auto term_freq_in_doc = item.second->in_doc.term_freq;
        auto doc_len = result->doc_info->title_len;
        auto rtd = _calc_bm25_r_factor(term_freq_in_query, term_freq_in_doc,
                                       doc_len, avg_doc_len);
#if 0
        LOG_DEBUG("wi=%f rtd=%f tf_in_q=%d tf_in_d=%d d_len=%d avg_d_len=%f",
                    wi, rtd,
                    term_freq_in_query, term_freq_in_doc,
                    doc_len, avg_doc_len);
#endif
        bm25 += wi * rtd;
    }
    result->bm25 = bm25;
    result->feature_mgr->add_feature("F_QU_BM25", result->bm25);
    return true;
}

float TinyEngine::_calc_bm25_r_factor(
        uint16_t term_freq_in_query,
        uint16_t term_freq_in_doc,
        uint32_t doc_len,
        float avg_doc_len) {
    float k1 = 2.0f;
    float k2 = 1.0f;
    float b = 0.75f;

    float Kd = k1 * (1 - b + b * static_cast<float>(doc_len) / avg_doc_len);
    auto left = static_cast<float>(term_freq_in_doc) * (k1 + 1) / (term_freq_in_doc + Kd);
    auto right = static_cast<float>(term_freq_in_query) * (k2 + 1) / (term_freq_in_query + k2);
    auto Rtd = left * right;
    return Rtd;
}

bool TinyEngine::_calc_cqr_ctr(std::shared_ptr<ResInfo> result) {
    float divisor = 0.0;
    for (const auto &item : result->match_term_map) {
        if (item.second->is_stopword) {
            continue;
        }
        divisor += item.second->hit_freq * item.second->idf;
    }
 
    auto dividend_cqr = query_info->wei_sum;
    auto dividend_ctr = result->doc_info->wei_sum;
    try {
        result->cqr = divisor / dividend_cqr;
        result->ctr = divisor / dividend_ctr;
    } catch (std::exception &e) {
        LOG_WARNING("query[%s] title[%s] dividend zero",
            query_info->query.c_str(), result->doc_info->title.c_str());
    }
    result->miss = 1 - result->cqr;

    result->feature_mgr->add_feature("F_QU_TERM_WEI_CQR", result->cqr);
    result->feature_mgr->add_feature("F_QU_TERM_WEI_CTR", result->ctr);
    result->feature_mgr->add_feature("F_QU_TERM_WEI_COVERAGE", result->cqr * result->ctr);
    result->feature_mgr->add_feature("F_QU_MISS", result->miss);
#if 0
    if (!GE_LOWER_AND_LE_UPPER(result->cqr, 0.0, 1.0)
            || !GE_LOWER_AND_LE_UPPER(result->ctr, 0.0, 1.0)) {

        LOG_WARNING("query[%s] title[%s] terms=%u cqr=%.2f/%.2f=%.2f ctr=%.2f/%.2f=%.2f",
             query_info->query.c_str(),
             result->doc_info->title.c_str(),
             result->term_hits,
             divisor,
             dividend_cqr,
             result->cqr,
             divisor,
             dividend_ctr,
             result->ctr);
    }
#endif
    return true;
}

void TinyEngine::_calc_order_overlap(std::shared_ptr<ResInfo> result) {
    // 所有特征均考虑先后序关系
    auto &req_terms = query_info->terms;
    auto &res_terms = result->doc_info->terms;
 
    // term粒度散乱有序命中
    auto term_lcseq = MathUtil::longest_common_subsequence(req_terms, res_terms);
    // term粒度连续命中
    auto term_lcstr = MathUtil::longest_continuous_substring(req_terms, res_terms);

    auto &query = query_info->query;
    auto &title = result->doc_info->title;
    // str粒度散乱有序命中
    auto str_lcseq = MathUtil::longest_common_subsequence(query, title);
    // str粒度连续命中
    auto str_lcstr = MathUtil::longest_continuous_substring(query, title);

    result->feature_mgr->add_feature("F_QU_TERM_LCSEQ", term_lcseq);
    result->feature_mgr->add_feature("F_QU_TERM_LCSEQ_OVER_Q",
                        static_cast<float>(term_lcseq) / req_terms.size());
    result->feature_mgr->add_feature("F_QU_TERM_LCSEQ_OVER_U",
                        static_cast<float>(term_lcseq) / res_terms.size());

    result->feature_mgr->add_feature("F_QU_TERM_LCSTR", term_lcstr);
    result->feature_mgr->add_feature("F_QU_TERM_LCSTR_OVER_Q",
                        static_cast<float>(term_lcstr) / req_terms.size());
    result->feature_mgr->add_feature("F_QU_TERM_LCSTR_OVER_U",
                        static_cast<float>(term_lcstr) / res_terms.size());

    result->feature_mgr->add_feature("F_QU_STR_LCSEQ", str_lcseq);
    result->feature_mgr->add_feature("F_QU_STR_LCSEQ_OVER_Q",
                        static_cast<float>(str_lcseq) / query.size());
    result->feature_mgr->add_feature("F_QU_STR_LCSEQ_OVER_U",
                        static_cast<float>(str_lcseq) / title.size());

    result->feature_mgr->add_feature("F_QU_STR_LCSTR", str_lcstr);
    result->feature_mgr->add_feature("F_QU_STR_LCSTR_OVER_Q",
                        static_cast<float>(str_lcstr) / query.size());
    result->feature_mgr->add_feature("F_QU_STR_LCSTR_OVER_U",
                        static_cast<float>(str_lcstr) / title.size());
}

void TinyEngine::_calc_scatter_overlap(std::shared_ptr<ResInfo> result) {
    uint32_t overlap = 0;
    // 只算命中,不考虑先后序关系
    for (const auto &item : result->match_term_map) { // <sign, freq_in_doc>
        auto pterm = table->get_term_info(item.first);
        overlap += pterm->term_len * item.second->hit_freq;
    }
    auto query_len = query_info->query_len;
    auto title_len = result->doc_info->title_len;
    auto extra = static_cast<float>(query_len - overlap) / (query_len + 1);
    auto str_len_cqr = static_cast<float>(overlap) / query_len;
    auto str_len_ctr = static_cast<float>(overlap) / title_len;
    auto str_len_coverage = str_len_cqr * str_len_ctr;

    result->extra = extra;
    result->feature_mgr->add_feature("F_QU_EXTRA", result->extra);

    result->feature_mgr->add_feature("F_Q_LEN", query_len);
    result->feature_mgr->add_feature("F_U_LEN", title_len);

    result->feature_mgr->add_feature("F_QU_STR_LEN_CQR", str_len_cqr);
    result->feature_mgr->add_feature("F_QU_STR_LEN_CTR", str_len_ctr);
    result->feature_mgr->add_feature("F_QU_STR_LEN_COVERAGE", str_len_coverage);

    auto hit_term_cnt = result->term_hits;
    auto query_term_cnt = query_info->terms.size();
    auto title_term_cnt = result->doc_info->terms.size();
    auto term_cnt_cqr = static_cast<float>(hit_term_cnt) / query_term_cnt;
    auto term_cnt_ctr = static_cast<float>(hit_term_cnt) / title_term_cnt;
    auto term_cnt_coverage = term_cnt_cqr * term_cnt_ctr;

    result->feature_mgr->add_feature("F_Q_TERM_CNT", query_term_cnt);
    result->feature_mgr->add_feature("F_U_TERM_CNT", title_term_cnt);
    //result->feature_mgr->add_feature("F_QU_HIT_TERM_CNT", hit_term_cnt);
    result->feature_mgr->add_feature("F_QU_TERM_CNT_CQR", term_cnt_cqr);
    result->feature_mgr->add_feature("F_QU_TERM_CNT_CTR", term_cnt_ctr);
    result->feature_mgr->add_feature("F_QU_TERM_CNT_COVERAGE", term_cnt_coverage);
}

void TinyEngine::_calc_distance(std::shared_ptr<ResInfo> result) {
    auto &req_terms = query_info->terms;
    auto &res_terms = result->doc_info->terms;

    auto term_edit_distance = MathUtil::edit_distance(req_terms, res_terms);
    auto str_edit_distance = MathUtil::edit_distance(query_info->query, result->doc_info->title);

    result->feature_mgr->add_feature("F_QU_TERM_EDIST", term_edit_distance);
    result->feature_mgr->add_feature("F_QU_TERM_EDIST_OVER_Q",
                        static_cast<float>(term_edit_distance) / req_terms.size());
    result->feature_mgr->add_feature("F_QU_TERM_EDIST_OVER_U",
                        static_cast<float>(term_edit_distance) / res_terms.size());
    result->feature_mgr->add_feature("F_QU_STR_EDIST", str_edit_distance);
    result->feature_mgr->add_feature("F_QU_STR_EDIST_OVER_Q",
                        static_cast<float>(str_edit_distance) / query_info->query_len);
    result->feature_mgr->add_feature("F_QU_STR_EDIST_OVER_U",
                        static_cast<float>(str_edit_distance) / result->doc_info->title_len);
}

void TinyEngine::_calc_disorder(std::shared_ptr<ResInfo> result) {
    EXPECT_GT_OR_RETURN(result->match_term_map.size(), 1, RETURN_ON_VOID);
    std::unordered_set<std::size_t> query_order_pair;
    auto &req_terms = query_info->terms;
    for (decltype(req_terms.size()) i = 0; i < req_terms.size(); ++i) {
        auto term_sign_i = req_terms[i].token_sign;

        if (result->match_term_map.count(term_sign_i) < 1) {
            continue;
        }
        for (decltype(req_terms.size()) j = i + 1; j < req_terms.size(); ++j) {
            auto term_sign_j = req_terms[j].token_sign;
            if (result->match_term_map.count(term_sign_j) < 1) {
                continue;
            }
            auto pair_sign_ij = _calc_pair_sign(term_sign_i, term_sign_j);
            query_order_pair.insert(pair_sign_ij);
        }
    }

    // LOG_DEBUG("hit_term_cnt=%llu, query_order_pair_cnt=%llu", 
    //         result->match_term_map.size(),
    //         query_order_pair.size());

    auto order_pair_cnt = 0;
    auto disorder_pair_cnt = 0;
    auto &res_terms = result->doc_info->terms;
    for (decltype(res_terms.size()) i = 0; i < res_terms.size(); ++i) {
        auto term_sign_i = res_terms[i].token_sign;
        if (result->match_term_map.count(term_sign_i) < 1) {
            continue;
        }
        for (decltype(res_terms.size()) j = i + 1; j < res_terms.size(); ++j) {
            auto term_sign_j = res_terms[j].token_sign;
            if (result->match_term_map.count(term_sign_j) < 1) {
                continue;
            }
            auto pair_sign_ij = _calc_pair_sign(term_sign_i, term_sign_j);
            if (query_order_pair.count(pair_sign_ij) > 0) {
                order_pair_cnt += 1;
            }
            auto pair_sign_ji = _calc_pair_sign(term_sign_j, term_sign_i);
            if (query_order_pair.count(pair_sign_ji) > 0) {
                disorder_pair_cnt += 1;
            }
        }
    }
    result->disorder = static_cast<float>(disorder_pair_cnt) / \
                       (order_pair_cnt + disorder_pair_cnt + 1);

    result->feature_mgr->add_feature("F_QU_DISORDER", result->disorder);
}

std::size_t TinyEngine::_calc_pair_sign(std::size_t term_sign_1, std::size_t term_sign_2) {
    std::size_t cut_sign = 0xffffffff;
    auto sign1 = term_sign_1 & cut_sign;
    auto sign2 = term_sign_2 & cut_sign;
    auto pair_sign = (sign1 << 32) | sign2;
    /*
    LOG_DEBUG("tsign1=%llu sign1=%llu tsign2=%llu sign2=%llu psign=%llu\n", 
            term_sign_1, sign1,
            term_sign_2, sign2,
            pair_sign);
    */
    return pair_sign;
}

void TinyEngine::_calc_diversity(std::shared_ptr<ResInfo> result) {
    std::vector<uint16_t> req_hit_offsets;
    std::vector<uint16_t> res_hit_offsets;
    for (const auto &item : result->match_term_map) {
        for (auto offset : item.second->in_query.offsets) {
            req_hit_offsets.push_back(offset);
        }
        for (auto offset : item.second->in_doc.offsets) {
            res_hit_offsets.push_back(offset);
        }
    }
    auto req_diversity = MathUtil::standard_variance(req_hit_offsets);
    auto res_diversity = MathUtil::standard_variance(res_hit_offsets);
    auto diversity = res_diversity - req_diversity;
    result->feature_mgr->add_feature("F_QU_HIT_DIVERSITY", diversity);
}

std::string TinyEngine::_title_highlight(std::shared_ptr<ResInfo> result) {
    std::string title;
    auto doc = result->doc_info;
    for (const auto &term : doc->terms) {
        if (result->match_term_map.count(term.token_sign) > 0) {
            if (result->match_term_map[term.token_sign]->is_syn_match
                    && !_is_highlight_syn_term) {
                title += term.token;
            } else {
                title += StrUtil::format("{}{}{}",
                    COLOR_RED, term.token.c_str(), COLOR_NONE);
            }
        } else {
            title += term.token;
        }
    }
    return title;
}

}; // end of namespace tiny_engine
/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
