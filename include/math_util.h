/*****************************************************************
*   Copyright (C) 2018 . All rights reserved.
*
*   @file:    math_util.h
*   @author:  marinyoung@163.com
*   @date:    2018/12/28 16:06:04
*   @brief  
*
*****************************************************************/

#ifndef __MATH_UTIL_H_
#define __MATH_UTIL_H_

#include "common.h"
#include <cmath>
#include <vector>
#include <algorithm>
#include <numeric> // accumulate
#include <iomanip> // setprecision
#include <sstream>

namespace tiny_engine {

class MathUtil {
public:
    /**
     * @brief 浮点数精度
     *        type: only suitble for float, double
     * @ref: https://blog.csdn.net/LaoJiu_/article/details/71703696
     */
    template <typename Type>
    static Type round(Type value, int bits);
    /**
     * @brief 向量的模
     */
    template <typename Type>
    static Type vec_module(const std::vector<Type> &vec);
    /**
     * @brief 向量余弦
     */
    template <typename Type>
    static Type cosine(
            const std::vector<Type> &first,
            const std::vector<Type> &second);
    /**
     * @brief 向量点积
     */
    template <typename Type>
    static Type dot_product(
            const std::vector<Type> &first,
            const std::vector<Type> &second);
    /**
     * @brief 向量的LP距离 (p >= 1)
     *      Type: built_in type, e.g. int, float, double, etc.
     */
    template <typename Type>
    static Type lp_distance(
            const std::vector<Type> &first,
            const std::vector<Type> &second,
            int p);
    /**
     * @brief 向量的欧拉距离
     */
    template <typename Type>
    static Type euclidean_distance(
            const std::vector<Type> &first,
            const std::vector<Type> &second);
    /**
     * @brief 向量的曼哈顿距离
     */
    template <typename Type>
    static Type manhattan_distance(
            const std::vector<Type> &first,
            const std::vector<Type> &second);
    /**
     * @brief 向量/字符串的编辑距离
     *      Type: 必须支持 .size()函数, 以及operator[], operator== 运算符
     */
    template <typename Type>
    static int edit_distance(
            const Type &first,
            const Type &second);
    /**
     * @brief 最长公共子序列（不连续）
     */
    template <typename Type>
    static int longest_common_subsequence(
            const Type &first,
            const Type &second);
    /**
     * @brief 最长公共子串（连续）
     */
    template <typename Type>
    static int longest_continuous_substring(
            const Type &first,
            const Type &second);
    /**
     * @brief 数组均值
     */
    template <typename Type>
    static double mean(const std::vector<Type> &array);
    /**
     * @brief 数组方差
     */
    template <typename Type>
    static double variance(const std::vector<Type> &array);
    /**
     * @brief 数组标准差
     */
    template <typename Type>
    static double standard_variance(const std::vector<Type> &array);
    /**
     * @brief 数组中位数
     *        会改变数组顺序; 数组大小为偶数时, 中位数为array[(array.size()-1)/2]
     * @param[in] array:数组
     * @return
     */
    template <typename Type>
    static Type median(std::vector<Type> &array);
    /**
     * @brief 计算tf
     * @param[in] term_freq_in_one_doc : term在某个doc中的频次
     * @return
     */
    static float calculate_tf(std::size_t term_freq_in_one_doc);
    /**
     * @brief 计算idf
     * @param[in] doc_num : 总doc数量（正排数量）
     * @param[in] term_freq_in_all_doc : term在所有doc中出现的总频次
     * @return:
     */
    static float calculate_idf(
            std::size_t term_freq_in_all_doc,
            std::size_t doc_num);
    /**
     * @brief sigmoid函数
     * @param[in] x: built_in type, int, float, double, etc
     * @return
     */
    template <typename Type>
    static float sigmoid(Type x);

private:
    DISALLOW_COPY_AND_ASSIGN(MathUtil);

private:
    MathUtil();
    ~MathUtil();
};

// -----------------------------------------------------------

template <typename Type>
Type MathUtil::round(Type value, int bits) {
    std::stringstream ss;
    ss << std::setprecision(bits) << value;
    ss >> value;
    return value;
}

template <typename Type>
Type MathUtil::vec_module(const std::vector<Type> &vec) {
    Type result = static_cast<Type>(0);
    for (const auto &item : vec) {
        result += pow(item, 2);
    }
    return sqrt(result);
}

template <typename Type>
Type MathUtil::cosine(
        const std::vector<Type> &first,
        const std::vector<Type> &second) {
    if (first.size() != second.size()) {
        throw "vector length unqeual";
        return Type();
    }
    Type result = static_cast<Type>(0);
    Type first_mode = static_cast<Type>(0);
    Type second_mode = static_cast<Type>(0);
    for (std::size_t i = 0; i < first.size(); ++i) {
        result += first[i] * second[i];
        first_mode += pow(first[i], 2);
        second_mode += pow(second[i], 2);
    }
    first_mode = sqrt(first_mode);
    second_mode = sqrt(second_mode);
    return result / (first_mode * second_mode);
}

template <typename Type>
Type MathUtil::dot_product(
        const std::vector<Type> &first,
        const std::vector<Type> &second) {
    if (first.size() != second.size()) {
        throw "vector length unqeual";
        return Type();
    }
    Type result = static_cast<Type>(0);
    for (std::size_t i = 0; i < first.size(); ++i) {
        result += first[i] * second[i];
    }
    return result;
}

template <typename Type>
Type MathUtil::lp_distance(
        const std::vector<Type> &first,
        const std::vector<Type> &second,
        int p) {
    if (first.size() != second.size()) {
        throw "vector length unqueal";
        return Type();
    }
    if (p < 1) {
        throw "param error: p < 1";
        return Type();
    }
    Type result = static_cast<Type>(0);
    for (std::size_t i = 0; i < first.size(); ++i) {
        result += pow(first[i] - second[i], p);
    }
    return pow(result, 1.0 / static_cast<float>(p));
}

template <typename Type>
Type MathUtil::euclidean_distance(
        const std::vector<Type> &first,
        const std::vector<Type> &second) {
    return lp_distance<Type>(first, second, 2);
}

template <typename Type>
Type MathUtil::manhattan_distance(
        const std::vector<Type> &first,
        const std::vector<Type> &second) {
    return lp_distance<Type>(first, second, 1);
}

template <typename Type>
int MathUtil::edit_distance(
        const Type &first,
        const Type &second) {
    const auto ROW = 1 + first.size();
    const auto COLUMN = 1 + second.size();
    std::vector<std::vector<int>> dp(ROW, std::vector<int>(COLUMN, 0));
    for (std::size_t i = 0; i < ROW; ++i) {
        dp[i][0] = i;
    }
    for (std::size_t j = 0; j < COLUMN; ++j) {
        dp[0][j] = j;
    }
    for (std::size_t i = 1; i < ROW; ++i) {
        for (std::size_t j = 1; j < COLUMN; ++j) {
            dp[i][j] = (first[i - 1] == second[j - 1] ? \
                        dp[i - 1][j - 1] : \
                        dp[i - 1][j - 1] + 1);

            dp[i][j] = std::min(dp[i][j],
                                std::min(dp[i - 1][j] + 1, dp[i][j - 1] + 1));
        }
    }
    return dp[ROW - 1][COLUMN - 1];
}

template <typename Type>
int MathUtil::longest_common_subsequence(
        const Type &first,
        const Type &second) {
    std::vector<std::vector<int>> dp(first.size() + 1,
                                    std::vector<int>(second.size() + 1, 0));
    for (decltype(first.size()) i = 1; i < first.size() + 1; ++i) {
        for (decltype(second.size()) j = 1; j < second.size() + 1; ++j) {
            if (first[i - 1] == second[j - 1]) {
                dp[i][j] = dp[i - 1][j - 1] + 1;
            } else {
                dp[i][j] = std::max(dp[i][j - 1], dp[i - 1][j]);
            }
        }
    }
    return dp[first.size()][second.size()];
}

template <typename Type>
int MathUtil::longest_continuous_substring(
        const Type &first,
        const Type &second) {
    int result = -1;
    std::vector<std::vector<int>> dp(first.size(), std::vector<int>(second.size(), 0));
    for (decltype(first.size()) i = 0; i < first.size(); ++i) {
        dp[i][0] = (first[i] == second[0] ? 1 : 0);
    }
    for (decltype(second.size()) j = 0; j < second.size(); ++j) {
        dp[0][j] = (first[0] == second[j] ? 1 : 0);
    }
    for (decltype(first.size()) i = 0; i < first.size(); ++i) {
        for (decltype(second.size()) j = 0; j < second.size(); ++j) {
            if (i < 1 || j < 1) {
                result = std::max(result, dp[i][j]);
                continue;
            }
            dp[i][j] = (first[i] == second[j] ? dp[i - 1][j - 1] + 1 : 0);
            result = std::max(dp[i][j], result);
        }
    }
    return result;
}

template <typename Type>
double MathUtil::mean(const std::vector<Type> &array) {
    EXPECT_FALSE_OR_RETURN(array.empty(), 0.0);
    return std::accumulate(array.begin(), array.end(), 0.0) / \
        static_cast<double>(array.size());
}

template <typename Type>
double MathUtil::variance(const std::vector<Type> &array) {
    EXPECT_FALSE_OR_RETURN(array.empty(), 0.0);
    auto mean = MathUtil::mean<Type>(array);
    double accumulate = 0.0;
    std::for_each(array.begin(), array.end(),
        [&accumulate, &mean](const Type &v) {
            accumulate += pow(v - mean, 2);
        });
    return accumulate / array.size();
}

template <typename Type>
double MathUtil::standard_variance(const std::vector<Type> &array) {
    return sqrt(MathUtil::variance(array));
}

template <typename Type>
Type MathUtil::median(std::vector<Type> &array) {
    EXPECT_FALSE_OR_RETURN(array.empty(), Type());
    auto mid_pos = array.size() / 2;
    auto mid_iter = array.begin();
    std::advance(array.begin(), mid_pos);
    std::nth_element(array.begin(), mid_iter, array.end());
    return array[mid_pos];
}

template <typename Type>
float MathUtil::sigmoid(Type x) {
    return 1.0 / (1 + exp(-x));
}

}; // end of namespace tiny_engine
#endif
/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
