/*****************************************************************
*   Copyright (C) 2018 . All rights reserved.
*   
*   @file:    math_util.cpp
*   @author:  marinyoung@163.com
*   @date:    2018/12/27 23:51:43
*   @brief:   
*
*****************************************************************/

#include "../include/math_util.h"

namespace tiny_engine {

float MathUtil::calculate_tf(std::size_t term_freq) {
    return static_cast<float>(term_freq);
}

float MathUtil::calculate_idf(std::size_t doc_freq, std::size_t doc_num) {
    auto divisor = static_cast<float>(doc_num - doc_freq) + 0.5;
    auto dividend = static_cast<float>(doc_freq) + 0.5;
    return log(divisor / dividend);
}

}; // end of namespace tiny_engine
/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
