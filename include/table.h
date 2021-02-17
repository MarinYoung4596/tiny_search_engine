/*****************************************************************
*   Copyright (C) 2018 . All rights reserved.
*
*   @file:    table.h
*   @author:  marinyoung@163.com
*   @date:    2018/11/20 21:08:49
*   @brief   
*
*****************************************************************/

#ifndef __TABLE_H_
#define __TABLE_H_

#include "common.h"
#include "str_util.h"
#include "log_util.h"
#include "time_util.h"
#include "file_util.h"
#include "config_util.h"
#include "time_util.h"
#include "math_util.h"
#include "node_def.h"
#include "segment.h"
#include "synonyms.h"
#ifdef XGBOOST
#include "xgboost_mgr.h"
#endif

#include <vector>
#include <unordered_set>
#include <unordered_map>
#include <string>
#include <algorithm>
#include <numeric> // accumulate
#include <memory> // shared_ptr
#include <fstream>
#include <cstring>

namespace tiny_engine {

class Table {
public:
    using MAX_FILE_PATH_LEN = std::integral_constant<int, 128>;
    using TermDocArrPair = std::pair<std::size_t, std::unordered_set<DocNode, DocNodeHash>>;

    Table();
    ~Table();
    bool init(std::shared_ptr<ConfigUtil> configs,
            std::shared_ptr<Segment> wordseg = nullptr);

    void set_fwd_path(const std::string &str);
    void set_inv_path(const std::string &str);
    void set_index_path(const std::string &str);

    std::size_t get_fwd_size() const;
    std::size_t get_inv_size() const;
    std::shared_ptr<DocInfo> get_doc_info(const std::size_t &doc_sign) const;
    std::shared_ptr<InvTermInfo> get_term_info(const std::size_t &term_sign) const;
    float get_term_idf(const std::size_t &term_sign) const;
    std::string get_term_txt(const std::size_t &term_sign) const;

    void calc_avg_doc_len();
    float get_avg_doc_len() const;

    bool hit_stopword(const std::size_t &term_sign) const;

    bool load_index_from_json();
    bool load_index_from_file(std::shared_ptr<Segment> wordseg);
    bool dump_index() const;

    bool add_index(std::shared_ptr<DocInfo> doc_info);
    bool delete_index(const std::size_t &doc_sign);

    /**
     * @brief 从倒排中召回
     * @param[in]:  query: query相关feature
     * @param[out]: result: 正排相关feature
     */
    bool recall(
            std::shared_ptr<QueryInfo> query_info,
            std::vector<std::shared_ptr<ResInfo>> &result) const;

    /**
     * @brief 填充 match_term_info
     * @param[in] term_node: 当前 term 在 query 中的命中情况
     * @param[in] term_info: 当前 term 在 倒排中的信息（e.g. idf 等）
     * @param[in] query_info: query其他信息
     * @param[in] doc_info: 当前 term 在 doc 中的信息
     * @param[out] match_term_info
     */
    bool fill_match_term_info(
            std::shared_ptr<InvTermInfo> term_info,
            std::shared_ptr<QueryInfo> query_info,
            std::shared_ptr<DocInfo> doc_info,
            std::shared_ptr<MatchTermInfo> mti) const;

    bool process_new_doc(
            std::shared_ptr<QueryInfo> query_info,
            std::shared_ptr<InvTermInfo> term_info,
            std::size_t doc_sign,
            uint16_t term_idx_in_q,
            bool is_syn_recall,
            std::unordered_map<std::size_t, std::shared_ptr<ResInfo>> &res_map) const;

private:
    DISALLOW_COPY_AND_ASSIGN(Table);
    /**
     * @brief 倒排结果归并
     * @param[in]  doc_array: 每个term对应的倒排拉链
     * @param[out] result: 归并后的结果拉链
     * @TODO 归并树归并
     */
    bool _merge(std::shared_ptr<QueryInfo> query_info,
            const std::vector<TermDocArrPair> &doc_array,
            std::vector<std::shared_ptr<ResInfo>> &result) const;

    bool _load_fwd_table();
    bool _load_inv_table();

    bool _load_stopword(const std::string &file);

    bool _dump_fwd_table() const;
    bool _dump_inv_table() const;

    /**
     * @brief 抓取的title有很多站点描述, 需要去除, 否则会影响相关性计算的准确性; e.g.
     *      q=有哪些你看一次笑一次的段子？
     *      title1=知乎上那些让你看一次笑一次的经典笑话段子! _360doc个人图书馆
     *      title2=有哪些段子或视频，你看一次笑一次？-原创-高清正版视频在线观看–爱奇艺
     */
    bool remove_site_description(const std::string &title, std::string &real_title);
    /**
     * @brief
     * @param[in] doc_info: doc相关feature
     * @param[in] update: 是否更新term wei, 适用于少量插入的场景, 否则影响建索引速度
     */
    bool _add_to_fwd_table(std::shared_ptr<DocInfo> doc_info, bool update_wei = false);
    /**
     * @brief
     * @param[in] doc_info: doc相关feature(已切词)
     * @param[in] update_idf: 是否更新idf, 适用于少量更新倒排的场景, 否则会影响建索引速度
     */
    bool _add_to_inv_table(std::shared_ptr<DocInfo> doc_info, bool update_idf = false);
    /**
     * @brief 更新所有倒排的idf, 适用于批量插入索引后调用, 以降低开销
     */
    bool _update_inv_table();
    /**
     * @brief 更新完倒排的idf后，更新正排所有doc的term wei
     */
    bool _update_fwd_table();

private:
    char* forward_file_path;
    char* invert_file_path;
    char* index_file_path;
    float avg_doc_len;
    int max_syn_term_recall;

private:
    // doc_sign, doc_info
    std::unordered_map<std::size_t, std::shared_ptr<DocInfo>> forward_table;
    // term_sign, term_info
    std::unordered_map<std::size_t, std::shared_ptr<InvTermInfo>> invert_table;
    // stopword set
    std::unordered_set<std::size_t> stopword;
};

class TinyEngine {
public:
    using MAX_INDEX_RECALL_NUM = std::integral_constant<int, 1000>;
    using MAX_SORT_NUM = std::integral_constant<int, 100>;
    using MAX_RESULT_NUM = std::integral_constant<int, 20>;
    using StrStrPair = std::pair<std::string, std::string>;

    TinyEngine(const std::string &config = "./conf/sys.conf",
            std::shared_ptr<Segment> wordseg = nullptr);

    bool init();

    bool stop();

    /**
     * @brief
     * @query[in] query
     * @result[out] relevent result <titles, url>
     */
    bool search(const std::string &query, std::vector<StrStrPair> &result);

private:
    DISALLOW_COPY_AND_ASSIGN(TinyEngine);

    bool _load_conf();
    /**
     * @brief 填充query feature, 主要是切词
     */
    bool _fill_query_info(const std::string &query);
    /**
     * @brief 从索引召回
     */
    bool _base_recall(std::vector<std::shared_ptr<ResInfo>> &results);

    /************* 计算结果级特征 *************/
    /**
     * @brief 获取query与doc的term交集
     */
    bool _get_intersections(
            std::shared_ptr<ResInfo> result,
            std::vector<TermNode> &target);
    /**
     * @brief 计算特征
     */
    bool _calc_features(std::shared_ptr<ResInfo> result);
    /**
     * @brief 计算主题匹配度
     */
    bool _calc_cqr_ctr(std::shared_ptr<ResInfo> result);
    /**
     * @brief
     */
    bool _calc_vsm(std::shared_ptr<ResInfo> result);
    /**
     * @brief 计算bm25
     */
    bool _calc_bm25(std::shared_ptr<ResInfo> result);

    float _calc_bm25_r_factor(uint16_t term_freq_in_query,
                              uint16_t term_freq_in_doc,
                              uint32_t doc_len,
                              float avg_doc_len);
    /**
     * @brief 有序命中
     */
    void _calc_order_overlap(std::shared_ptr<ResInfo> result);
    /**
     * @brief 散乱命中
     */
    void _calc_scatter_overlap(std::shared_ptr<ResInfo> result);
    /**
     * @brief 以term为粒度计算编辑距离、偏移距离
     */
    void _calc_distance(std::shared_ptr<ResInfo> result);
    /**
     * @brief 计算数组的最短距离
     */
    int _smallest_distance(
            const std::vector<uint32_t> &left,
            const std::vector<uint32_t> &right);
    /**
     * @brief 计算disorder
     */
    void _calc_disorder(std::shared_ptr<ResInfo> result);
    std::size_t _calc_pair_sign(std::size_t term_sign_i, std::size_t term_sign_j);
    /*
     * @brief 计算散乱命中情况
     */
    void _calc_diversity(std::shared_ptr<ResInfo> result);
    /**
     * @brief 结果排序
     */
    bool _rank_results();

    /**
     * @brief title飘红
     */
    std::string _title_highlight(std::shared_ptr<ResInfo> result);

private:
    // 一些阈值参数
    int _max_index_recall_num;
    int _max_2nd_sort_num;
    int _max_result_num;
    bool _is_highlight_syn_term;

    const float EPSILON = std::numeric_limits<float>::epsilon();

private:
    std::shared_ptr<ConfigUtil> configs;
    std::shared_ptr<Segment> wordseg;
    std::shared_ptr<Synonyms> wordsyn;
    std::unique_ptr<Table> table;
#ifdef XGBOOST
    std::unique_ptr<XGBoostMgr> xgb_mgr;
#endif
    std::shared_ptr<QueryInfo> query_info; // 请求query feature特征
    std::vector<std::shared_ptr<ResInfo>> results_info; // 召回的doc list
};

}; // end of namespace tiny_engine

#endif // __TABLE_H_

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
