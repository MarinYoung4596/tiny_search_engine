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

namespace tiny_engine {

DocInfo::DocInfo(
        std::size_t sign,
        const std::string &t,
        const std::string &u,
        const std::vector<TermNode> &tms,
        const TermFreqMap &tf_map,
        float module) :
    doc_sign(sign),
    title(t),
    url(u),
    terms(tms),
    term_freq_map(tf_map),
    vec_module(module) {
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
        uint32_t len,
        uint32_t tf,
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
    edit_distance(0),
    offset_distance(0),
    edit_ratio(0.0),
    offset_ratio(0.0),
    final_score(0.0) {
    update_res_info();
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

// -----------------------------------------------------------

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
        LOG_DEBUG("index updated ok, cost %lu ms", delta_time);
#endif
    }

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

inline std::shared_ptr<TermInfo> Table::get_term_info(const std::size_t &term_sign) const {
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
    LOG_INFO("load file[%s] succeed, len[%lu]", file.c_str(), stopword.size());
    return true;
}

inline bool Table::hit_stopword(const std::size_t &term_sign) const {
    return stopword.find(term_sign) != stopword.end();
}

bool Table::load_index_from_file(std::shared_ptr<Segment> wordseg) {
    EXPECT_TRUE_OR_RETURN_LOGGED(
            FileUtil::is_readable(index_file_path),
            false, "file not exist");
#ifdef DEBUG
    auto time_begin = TimeUtil::get_curr_timeval();
#endif
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
        TermFreqMap term_freq_map;
        if (!(wordseg->get_token(title, tokens, &term_freq_map))) {
            LOG_WARNING("segment title[%s] failed", title.c_str());
            continue;
        }
        auto doc_sign = StrUtil::str_to_sign(url); /////
        std::shared_ptr<DocInfo> doc_info(
                new DocInfo(doc_sign, title, url, tokens, term_freq_map));
        if (!add_index(doc_info)) {
            LOG_WARNING("index build error, query[%s] title[%s]",
                    vec[0].c_str(), title.c_str());
        }
    }
    ifs.close();
#ifdef DEBUG
    auto time_end = TimeUtil::get_curr_timeval();
    auto delta_time = TimeUtil::timeval_diff_ms(&time_end, &time_begin);
    LOG_INFO("table initialized ok, fwd=%lu inv=%lu cost=%lu ms",
            get_fwd_size(), get_inv_size(), delta_time);
#endif
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
	EXPECT_TRUE_OR_RETURN_LOGGED(FileUtil::generate_md5_file(invert_file_path),
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
        std::shared_ptr<TermInfo> term_info = nullptr;
        if (invert_table.find(term_sign) != invert_table.end()) {
            term_info = invert_table[term_sign];
        } else {
            term_info = std::make_shared<TermInfo>(it->token_sign, it->token, it->length);
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
        auto module = 0.0;
        for (auto &term : item.second->terms) {
            if (hit_stopword(term.token_sign)) {
                continue;
            }
            auto idf = get_term_idf(term.token_sign);
            auto tf = term.dup;
            term.wei = tf * idf;
            module += pow(term.wei, 2.0);
        }
        item.second->vec_module = sqrt(module);
    }
    return true;
}

bool Table::recall(std::shared_ptr<QueryInfo> query_info,
        std::vector<std::shared_ptr<ResInfo>> &result) const {
    if (!result.empty()) {
        result.clear();
    }
    // <term_sign, doc_list>
    std::vector<TermDocArrPair> doc_array;
    auto i = -1;
    for (const auto &term : query_info->terms) {
        i += 1;
        if (term.dup > 1) {
            continue; // 去除重复
        }
        // TODO 用同义改写term去召回
        auto term_info = get_term_info(term.token_sign);
        if (CHECK_NULL(term_info)) {
            continue;
        }
        /*
        auto &doc_list = term_info->docs;
        doc_array.push_back(std::make_pair(term.token_sign, doc_list));
        */
        //// TMP
        auto doc_list = term_info->docs;
        auto &syn_terms = query_info->syns[i];
        for (const auto &t : syn_terms) {
            if (t == term.token) {
                continue;
            }
            auto term_sign = StrUtil::str_to_sign(t);
            auto syn_term_info = get_term_info(term_sign);
            if (CHECK_NULL(syn_term_info)) {
                continue;
            }
            auto len_begin = doc_list.size(); //
            for (const auto &doc : syn_term_info->docs) {
                doc_list.insert(doc);
            }
            auto len_end = doc_list.size(); //
#ifdef DEBUG
            LOG_DEBUG("term[%s] recall[be4=%u after=%u]", t.c_str(), len_begin, len_end);
#endif
        }
        doc_array.push_back(std::make_pair(term.token_sign, doc_list));
        ////// TMP ended
    }
    _merge(query_info, doc_array, result);

    // 1st_sort: sort by term_hits
    std::sort(result.begin(), result.end(),
            [&](const std::shared_ptr<ResInfo> &lhs, const std::shared_ptr<ResInfo> &rhs) {
        if (lhs->term_hits != rhs->term_hits) {
            return lhs->term_hits > rhs->term_hits;
        }
        return lhs->doc_info->title.size() < rhs->doc_info->title.size();
    });
    return true;
}

bool Table::_merge(std::shared_ptr<QueryInfo> query_info,
        const std::vector<TermDocArrPair> &doc_array,
        std::vector<std::shared_ptr<ResInfo>> &result) const {
    EXPECT_FALSE_OR_RETURN(doc_array.empty(), false);

    if (!result.empty()) {
        result.clear();
    }
    // query_doc_intersection_terms:
    //            doc_sign, <term_sign, freq>
    std::unordered_map<std::size_t, TermFreqMap> docs;
    // doc_array: term_sign, docs_list
    for (const auto &item : doc_array) { // row: each term
        auto &term_sign = item.first;
        for (const auto &doc_node : item.second) { // column: each doc
            auto &doc_sign = doc_node.doc_sign;
            auto doc_iter = docs.find(doc_sign);
            if (doc_iter == docs.end()) {
                TermFreqMap intersection_terms;
                intersection_terms[term_sign] = 1;
                docs[doc_sign] = intersection_terms;
            } else {
                auto &intersection_terms = doc_iter->second;
                intersection_terms[term_sign] = 1;
            }
        }
    }
    // update fwd info
    for (auto &doc : docs) {
        auto &doc_sign = doc.first;
        auto doc_info = get_doc_info(doc_sign);
        if (CHECK_NULL(doc_info)) {
            LOG_ERROR("doc[%lu] not found in forward_table!", doc_sign);
            continue;
        }
        // update: <Query与Doc的term交集, term在query\doc中出现的最小频次>
        for (auto &term : doc.second) {
            auto &term_sign = term.first;
            auto &term_freq_in_query = query_info->term_freq_map[term_sign];
            auto &term_freq_in_doc = doc_info->term_freq_map[term_sign];
            term.second = std::min(term_freq_in_query, term_freq_in_doc);
        }
        auto res_node = std::make_shared<ResInfo>(doc_sign, doc_info, doc.second);
        result.push_back(res_node);
    }
    return true;
}

// -----------------------------------------------------------

TinyEngine::TinyEngine(const std::string &config,
        std::shared_ptr<Segment> _wordseg) :
    wordseg(_wordseg) {
    configs = std::make_shared<ConfigUtil>(config);
    query_info = std::make_shared<QueryInfo>();
    if (CHECK_NULL(wordseg)) {
        wordseg = std::make_shared<Segment>();
    }
    wordsyn = std::make_shared<Synonyms>();
    table.reset(new Table());
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
    // load conf
    EXPECT_TRUE_OR_RETURN_LOGGED(_load_conf(), false, "load conf error");
    // load dict

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
        auto &url = res->doc_info->url;
#ifndef DEBUG
        std::cout << StrUtil::format("{}\t{}\n", query, title);
#endif

#ifdef DEBUG
        std::cout << StrUtil::format(
                "{}\t{}\t{}\tscore={}," \
                "overlap={},qlen={},tlen={},nqterm={},ntterm={},thits={}," \
                "cqr={},ctr={},vsm={},edist={},odist={},lcons={},lcoms={}\n",
                query,
                res->doc_info->url, res->doc_info->title,
                res->final_score,
                res->str_overlap_len,
                query_info->query_len,
                res->doc_info->title_len,
                query_info->terms.size(),
                res->doc_info->terms.size(),
                res->term_hits,
                MathUtil::round(res->cqr, 2),
                MathUtil::round(res->ctr, 2),
                MathUtil::round(res->vsm, 2),
                res->edit_distance,
                res->offset_distance,
                res->longest_continuous_substr,
                res->longest_common_subseq
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
    TermFreqMap term_freq_map;
    // word segment
    if (!(wordseg->get_token(query, tokens, &term_freq_map))) {
        LOG_WARNING("query[%s] segment error", query.c_str());
        return false;
    }
    // calculate tf * idf
    auto module = 0.0;
    for (auto it = tokens.begin(); it != tokens.end(); ++it) {
        if (table->hit_stopword(it->token_sign)) {
            continue;
        }
        auto idf = table->get_term_idf(it->token_sign);
        auto tf = it->dup;
        it->wei = idf * sqrt(tf) / sqrt(tokens.size());
        module += pow(it->wei, 2.0);
    }
    query_info->init();
    query_info->query = query;
    query_info->terms = std::move(tokens);
	query_info->term_freq_map = std::move(term_freq_map);
    query_info->vec_module = sqrt(module);
    if (query_info->terms.size() > 0) {
        auto p_last_term = std::prev(query_info->terms.end());
        query_info->query_len = p_last_term->offset + p_last_term->length;
    }
    ////// TMP
    std::vector<std::string> syns;
    for (const auto &term : query_info->terms) {
        wordsyn->get_syns(term.token, syns);
        query_info->syns.push_back(syns);
        syns.clear();
    }
    ///// TMP end

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
    auto i = 0;
    for (auto &res : results_info) {
        if (i >= _max_2nd_sort_num) {
            break;
        }
        _calculate_features(res);
        ++i;
    }
    std::sort(results_info.begin(), results_info.end(),
        [&](const std::shared_ptr<ResInfo> &lhs, const std::shared_ptr<ResInfo> &rhs) {
            return lhs->final_score > rhs->final_score;
        });
    return true;
}

void TinyEngine::_calculate_features(std::shared_ptr<ResInfo> result) {
    EXPECT_NE_OR_RETURN(nullptr, result, RETURN_ON_VOID);

    result->vsm = _vsm(result);
    _calculate_cqr_ctr(result);
    _calculate_overlap(result);
    _calculate_term_overlap(result);
    _calculate_distance(result);

    result->final_score = result->vsm;
    //result->final_score = result->cqr * result->ctr;
}

bool TinyEngine::_get_intersections(
        std::shared_ptr<ResInfo> result,
        std::vector<TermNode> &target) {
    EXPECT_NE_OR_RETURN(nullptr, result, false);
    if (!target.empty()) {
        target.clear();
    }
    for (const auto &item : result->hit_term_map) {
        auto pterm = table->get_term_info(item.first);
        // offset: 这里用不上, 设为0
        TermNode node(item.first, pterm->term_txt, 0, pterm->term_len,
                      item.second, pterm->idf * item.second);
        target.push_back(std::move(node));
    }
    return true;
}

float TinyEngine::_vsm(std::shared_ptr<ResInfo> result) {
    EXPECT_FALSE_OR_RETURN(nullptr == result || result->hit_term_map.empty(), 0.0);
    auto &req_vec_module = query_info->vec_module;
    auto &res_vec_module = result->doc_info->vec_module;
    EXPECT_GT_OR_RETURN(abs(req_vec_module - 0.0), EPSILON, 0.0);
    EXPECT_GT_OR_RETURN(abs(res_vec_module - 0.0), EPSILON, 0.0);

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
    // TODO: 同义词命中补充

    std::vector<float> req_term_vec;
    std::vector<float> res_term_vec;
    for (auto it = term_wei_map.begin(); it != term_wei_map.end(); ++it) {
        req_term_vec.push_back(it->second.first);
        res_term_vec.push_back(it->second.second);
    }
    return MathUtil::dot_product(req_term_vec, res_term_vec) / (req_vec_module * res_vec_module);
}

bool TinyEngine::_calculate_cqr_ctr(std::shared_ptr<ResInfo> result) {
    float divisor = 0.0;
	for (const auto &item : result->hit_term_map) {
        if (table->hit_stopword(item.first)) {
            continue;
        }
		divisor += item.second * table->get_term_idf(item.first);
	}
    std::unordered_set<std::size_t> term_set;
    auto &req_terms = query_info->terms;
    float dividend_cqr = 0.0;
    for (auto rit = req_terms.crbegin(); rit != req_terms.crend(); ++rit) {
        if (table->hit_stopword(rit->token_sign)) {
            continue;
        }
        if (term_set.find(rit->token_sign) != term_set.end()) {
            continue;
        }
        dividend_cqr += rit->wei;
        term_set.insert(rit->token_sign);
    }

    term_set.clear();
    auto &res_terms = result->doc_info->terms;
    float dividend_ctr = 0.0;
    for (auto rit = res_terms.crbegin(); rit != res_terms.crend(); ++rit) {
        if (table->hit_stopword(rit->token_sign)) {
            continue;
        }
        if (term_set.find(rit->token_sign) != term_set.end()) {
            continue;
        }
        dividend_ctr += rit->wei;
        term_set.insert(rit->token_sign);
    }

    try {
        result->cqr = divisor / dividend_cqr;
        result->ctr = divisor / dividend_ctr;
    } catch (std::exception &e) {
        LOG_WARNING("query[%s] url[%s] zero",
            query_info->query.c_str(), result->doc_info->title.c_str());
    }
// #ifdef DEBUG
//     if (result->cqr < 0.0
//             || result->cqr > 1.0
//             || result->ctr < 0.0
//             || result->ctr > 1.0) {

//         std::vector<TermNode> intersections;
//         _get_intersections(result, intersections);

//         std::vector<std::string> vec;
//         for_each(intersections.begin(), intersections.end(),
//                 [&vec](const TermNode &node) {
//                     vec.push_back(node.to_str());
//                 });
//         std::string term_str = StrUtil::join(vec.begin(), vec.end(), " | ");
//         LOG_WARNING("query[%s] title[%s] terms[%s] cqr=%.2f/%.2f=%.2f ctr=%.2f/%.2f=%.2f",
//              query_info->query.c_str(),
//              result->doc_info->title.c_str(),
//              term_str.c_str(),
//              divisor,
//              dividend_cqr,
//              result->cqr,
//              divisor,
//              dividend_ctr,
//              result->ctr);
//     }
// #endif
    return true;
}

void TinyEngine::_calculate_term_overlap(std::shared_ptr<ResInfo> result) {
    auto &req_terms = query_info->terms;
    auto &res_terms = result->doc_info->terms;

    result->longest_common_subseq = MathUtil::longest_common_subsequence(req_terms, res_terms);
    result->longest_continuous_substr = MathUtil::longest_continuous_substring(req_terms, res_terms);
}

void TinyEngine::_calculate_overlap(std::shared_ptr<ResInfo> result) {
    int overlap = 0;
    for (const auto &item : result->hit_term_map) { // <sign, freq_in_doc>
        auto pterm = table->get_term_info(item.first);
        overlap += pterm->term_len * item.second;
    }
    result->str_overlap_len = overlap;
}

void TinyEngine::_calculate_distance(std::shared_ptr<ResInfo> result) {
    auto &req_terms = query_info->terms;
    auto &res_terms = result->doc_info->terms;

    result->edit_distance = MathUtil::edit_distance(req_terms, res_terms);

    auto req_offsets_dist = _offset_distance_helper(req_terms, result->hit_term_map);
    auto res_offsets_dist = _offset_distance_helper(res_terms, result->hit_term_map);
// #ifdef DEBUG
//     LOG_DEBUG("query[%s] title[%s] req[%d] res[%d]",
//             query_info->query.c_str(),
//             result->doc_info->title.c_str(),
//             req_offsets_dist,
//             res_offsets_dist);
// #endif
    result->offset_distance = abs(req_offsets_dist - res_offsets_dist);
}

int TinyEngine::_offset_distance_helper(
        const std::vector<TermNode> &terms,
        const TermFreqMap &target_map) {
    // <sign, [offset list]>
    std::unordered_map<std::size_t, std::vector<uint32_t>> term_offsets;
    for (const auto &term : terms) {
        // 从前往后，保证有序
        auto iter = term_offsets.find(term.token_sign);
        if (iter != term_offsets.end()) {
            iter->second.push_back(term.offset);
        } else {
            term_offsets[term.token_sign] = std::vector<uint32_t>({term.offset});
        }
    }
    // FIXME: 同一个term, query与doc中命中的次数不一样，会重复计算，导致offset_diff不准
    // 1) 会跟自身算偏移距离 2) <A,B> <B,A>重复计算
    auto offset_diff = 0;
    std::unordered_set<std::size_t> term_pair_set;
    std::unordered_set<std::size_t> cur_term_set;
    for (decltype(terms.size()) i = 0; i < terms.size(); ++i) {
        auto &cur_term_sign = terms[i].token_sign;
        if (!target_map.count(cur_term_sign)) {
            continue;
        }
        if (cur_term_set.count(cur_term_sign) > 0) {
            continue;
        }
        std::unordered_set<std::size_t> next_term_set;
        for (auto j = i + 1; j < terms.size(); ++j) {
            auto &next_term_sign = terms[j].token_sign;
            if (!target_map.count(next_term_sign)) {
                continue;
            }
            if (next_term_set.count(next_term_sign) > 0) {
                continue;
            }
            auto term_pair_sign = cur_term_sign & next_term_sign;
            if (term_pair_set.count(term_pair_sign) > 0) {
                continue;
            }
            offset_diff += _smallest_distance(
                                term_offsets[cur_term_sign],
                                term_offsets[next_term_sign]);
            next_term_set.insert(next_term_sign);
            term_pair_set.insert(term_pair_sign);
        }
        cur_term_set.insert(cur_term_sign);
    }
    return offset_diff;
}

int TinyEngine::_smallest_distance(
        const std::vector<uint32_t> &left,
        const std::vector<uint32_t> &right) {
    auto minimum = std::numeric_limits<uint32_t>::max();
    decltype(left.size()) i = 0;
    decltype(right.size()) j = 0;
    while (i < left.size() && j < right.size()) {
        if (left[i] >= right[j]) {
            minimum = std::min(minimum, left[i] - right[j]);
            ++j;
        } else {
            minimum = std::min(minimum, right[j] - left[i]);
            ++i;
        }
    }
    return minimum;
}

std::string TinyEngine::_title_highlight(std::shared_ptr<ResInfo> result) {
    std::string title;
    auto doc = result->doc_info;
    for (const auto &term : doc->terms) {
        if (result->hit_term_map.count(term.token_sign) > 0) {
            title += StrUtil::format("{}{}{}",
                COLOR_RED, term.token.c_str(), COLOR_NONE);
        } else {
            title += term.token;
        }
    }
    return std::move(title);
}

}; // end of namespace tiny_engine
/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
