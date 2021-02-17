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
     * @brief �ӵ������ٻ�
     * @param[in]:  query: query���feature
     * @param[out]: result: �������feature
     */
    bool recall(
            std::shared_ptr<QueryInfo> query_info,
            std::vector<std::shared_ptr<ResInfo>> &result) const;

    /**
     * @brief ��� match_term_info
     * @param[in] term_node: ��ǰ term �� query �е��������
     * @param[in] term_info: ��ǰ term �� �����е���Ϣ��e.g. idf �ȣ�
     * @param[in] query_info: query������Ϣ
     * @param[in] doc_info: ��ǰ term �� doc �е���Ϣ
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
     * @brief ���Ž���鲢
     * @param[in]  doc_array: ÿ��term��Ӧ�ĵ�������
     * @param[out] result: �鲢��Ľ������
     * @TODO �鲢���鲢
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
     * @brief ץȡ��title�кܶ�վ������, ��Ҫȥ��, �����Ӱ������Լ����׼ȷ��; e.g.
     *      q=����Щ�㿴һ��Цһ�εĶ��ӣ�
     *      title1=֪������Щ���㿴һ��Цһ�εľ���Ц������! _360doc����ͼ���
     *      title2=����Щ���ӻ���Ƶ���㿴һ��Цһ�Σ�-ԭ��-����������Ƶ���߹ۿ��C������
     */
    bool remove_site_description(const std::string &title, std::string &real_title);
    /**
     * @brief
     * @param[in] doc_info: doc���feature
     * @param[in] update: �Ƿ����term wei, ��������������ĳ���, ����Ӱ�콨�����ٶ�
     */
    bool _add_to_fwd_table(std::shared_ptr<DocInfo> doc_info, bool update_wei = false);
    /**
     * @brief
     * @param[in] doc_info: doc���feature(���д�)
     * @param[in] update_idf: �Ƿ����idf, �������������µ��ŵĳ���, �����Ӱ�콨�����ٶ�
     */
    bool _add_to_inv_table(std::shared_ptr<DocInfo> doc_info, bool update_idf = false);
    /**
     * @brief �������е��ŵ�idf, �����������������������, �Խ��Ϳ���
     */
    bool _update_inv_table();
    /**
     * @brief �����굹�ŵ�idf�󣬸�����������doc��term wei
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
     * @brief ���query feature, ��Ҫ���д�
     */
    bool _fill_query_info(const std::string &query);
    /**
     * @brief �������ٻ�
     */
    bool _base_recall(std::vector<std::shared_ptr<ResInfo>> &results);

    /************* ������������ *************/
    /**
     * @brief ��ȡquery��doc��term����
     */
    bool _get_intersections(
            std::shared_ptr<ResInfo> result,
            std::vector<TermNode> &target);
    /**
     * @brief ��������
     */
    bool _calc_features(std::shared_ptr<ResInfo> result);
    /**
     * @brief ��������ƥ���
     */
    bool _calc_cqr_ctr(std::shared_ptr<ResInfo> result);
    /**
     * @brief
     */
    bool _calc_vsm(std::shared_ptr<ResInfo> result);
    /**
     * @brief ����bm25
     */
    bool _calc_bm25(std::shared_ptr<ResInfo> result);

    float _calc_bm25_r_factor(uint16_t term_freq_in_query,
                              uint16_t term_freq_in_doc,
                              uint32_t doc_len,
                              float avg_doc_len);
    /**
     * @brief ��������
     */
    void _calc_order_overlap(std::shared_ptr<ResInfo> result);
    /**
     * @brief ɢ������
     */
    void _calc_scatter_overlap(std::shared_ptr<ResInfo> result);
    /**
     * @brief ��termΪ���ȼ���༭���롢ƫ�ƾ���
     */
    void _calc_distance(std::shared_ptr<ResInfo> result);
    /**
     * @brief �����������̾���
     */
    int _smallest_distance(
            const std::vector<uint32_t> &left,
            const std::vector<uint32_t> &right);
    /**
     * @brief ����disorder
     */
    void _calc_disorder(std::shared_ptr<ResInfo> result);
    std::size_t _calc_pair_sign(std::size_t term_sign_i, std::size_t term_sign_j);
    /*
     * @brief ����ɢ���������
     */
    void _calc_diversity(std::shared_ptr<ResInfo> result);
    /**
     * @brief �������
     */
    bool _rank_results();

    /**
     * @brief titleƮ��
     */
    std::string _title_highlight(std::shared_ptr<ResInfo> result);

private:
    // һЩ��ֵ����
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
    std::shared_ptr<QueryInfo> query_info; // ����query feature����
    std::vector<std::shared_ptr<ResInfo>> results_info; // �ٻص�doc list
};

}; // end of namespace tiny_engine

#endif // __TABLE_H_

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
