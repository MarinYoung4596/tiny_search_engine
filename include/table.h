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
#include "segment.h"
#include "synonyms.h"

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

// for forward table, Query
class TermNode;
using TermFreqMap = std::unordered_map<std::size_t, uint16_t>;

// ���� doc �ڵ�
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
    std::size_t doc_sign;
    std::string title;
    uint32_t title_len; // unicode length
    std::string url;
    std::vector<TermNode> terms; // title�д�
    TermFreqMap term_freq_map; // title��ÿ��term��Ƶ��
    float vec_module; // terms tf*idf ������ģ
};

// ���� doc �ڵ�; ����չ
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

// ���� term �ڵ�
class TermInfo {
public:
    TermInfo() {}
    TermInfo(std::size_t sign,
            const std::string &t,
            uint32_t len = 0,
            uint32_t tf = 0,
            float idf = 0.0);
public:
    std::size_t term_sign;
    std::string term_txt;
    uint32_t term_len; // term�ĳ���
    uint32_t term_freq; // term������doc�г��ֵ��ܴ���
    float idf;
    std::unordered_set<DocNode, DocNodeHash> docs;
};

// query���feature
class QueryInfo {
public:
    QueryInfo() {}
    QueryInfo(const std::string &q);
    void init();
    std::string to_str() const;
public:
    std::string query;
    uint32_t query_len; // unicode length
    std::vector<TermNode> terms; // query�д�
	TermFreqMap term_freq_map; // ÿ��term�ĳ���Ƶ��: <sign, freq>
    float vec_module; // terms tf*idf ������ģ
    std::vector<std::vector<std::string>> syns; /////// TMP
};

// doc���feature, ����query��doc�Ĺ����̶�
class ResInfo {
public:
    ResInfo() {}
    ResInfo(const std::size_t &sign,
            std::shared_ptr<DocInfo> doc,
            const TermFreqMap &hit_terms);
    void update_res_info();
    bool operator < (const ResInfo &rhs) const;

public:
    std::size_t doc_sign;
    std::shared_ptr<DocInfo> doc_info;
    TermFreqMap hit_term_map; // doc��query��term����: <sign, term��query,doc�е���СƵ��>
    TermFreqMap hit_syn_map; // query termͬ�������doc�����: <sign, ͬ���term��query�е�index>
    uint16_t term_hits; // doc��query��term��������(�����ظ�)

    // qu����Դ�ֵ�����
    float vsm;
    float cqr;
    float ctr;
    int str_overlap_len;
    int edit_distance;
    int offset_distance;
    int longest_common_subseq;
    int longest_continuous_substr;
    float edit_ratio;
    float offset_ratio;

    // rank score
    float final_score;
};

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
    std::shared_ptr<TermInfo> get_term_info(const std::size_t &term_sign) const;
    float get_term_idf(const std::size_t &term_sign) const;
    std::string get_term_txt(const std::size_t &term_sign) const;

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
    bool recall(std::shared_ptr<QueryInfo> query_info,
            std::vector<std::shared_ptr<ResInfo>> &result) const;

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
    // doc_sign, doc_info
    std::unordered_map<std::size_t, std::shared_ptr<DocInfo>> forward_table;
    // term_sign, term_info
    std::unordered_map<std::size_t, std::shared_ptr<TermInfo>> invert_table;

    char* forward_file_path;
    char* invert_file_path;
    char* index_file_path;

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
    void _calculate_features(std::shared_ptr<ResInfo> result);
    /**
     * @brief ��������ƥ���
     */
    bool _calculate_cqr_ctr(std::shared_ptr<ResInfo> result);
    /**
     * @brief
     */
    float _vsm(std::shared_ptr<ResInfo> result);
    /**
     * @brief ��termΪ���ȼ�������������С�������Ӵ�����
     */
    void _calculate_term_overlap(std::shared_ptr<ResInfo> result);
    /**
     * @brief �ַ���overlap�ľ���
     */
    void _calculate_overlap(std::shared_ptr<ResInfo> result);
    /**
     * @brief ��termΪ���ȼ���༭���롢ƫ�ƾ���
     */
    void _calculate_distance(std::shared_ptr<ResInfo> result);
    int _offset_distance_helper(
            const std::vector<TermNode> &terms,
            const TermFreqMap &target_map);
    /**
     * @brief �����������̾���
     */
    int _smallest_distance(
            const std::vector<uint32_t> &left,
            const std::vector<uint32_t> &right);
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

    const float EPSILON = std::numeric_limits<float>::epsilon();

private:
    std::shared_ptr<ConfigUtil> configs;
    std::shared_ptr<Segment> wordseg;
    std::shared_ptr<Synonyms> wordsyn; // TMP
    std::unique_ptr<Table> table;

    std::shared_ptr<QueryInfo> query_info; // ����query feature����
    std::vector<std::shared_ptr<ResInfo>> results_info; // �ٻ�url list
};

}; // end of namespace tiny_engine

#endif // __TABLE_H_

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
