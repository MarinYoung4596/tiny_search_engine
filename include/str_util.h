/*****************************************************************
*   Copyright (C) 2018 . All rights reserved.
*   
*   @file:    str_util.h
*   @author:  marinyoung@163.com
*   @date:    2018/11/22 22:52:42
*   @brief  
*
*****************************************************************/

#ifndef __STR_UTIL_H_
#define __STR_UTIL_H_

#include "common.h"
#include <vector>
#include <string>
#include <sstream>
#include <algorithm>
#include <functional>
#include <locale>
//#include <codecvt>
#include <cstdarg> // va_args
#include <cstring>

namespace tiny_engine {

class StrUtil {
public:
    static std::string left_trim(const std::string &str);
    static std::string right_trim(const std::string &str);
    static std::string trim(const std::string &str);
    static std::string left_trim_with(const std::string &str, char ch);
    static std::string right_trim_with(const std::string &str, char ch);
    static std::string trim_with(const std::string &str, char ch);

    static std::string to_lower_case(const std::string &str);
    static std::string to_upper_case(const std::string &str);

    static bool is_digital_str(const std::string &str);
    static bool is_alpha_str(const std::string &str);
    static bool is_alnum_str(const std::string &str);
    static bool is_space_str(const std::string &str);
    static bool is_en_str(const std::string &str);
    static bool is_zh_str(const std::string &str);
    static bool is_en_punct_str(const std::string &str);
    static bool is_zh_punct_str(const std::string &str);
    static bool is_lower_str(const std::string &str);
    static bool is_upper_str(const std::string &str);

    static bool is_start_with(const std::string &str, const std::string &prefix);
    static bool is_end_with(const std::string &str, const std::string &postfix);

	static std::string wstr_to_str(const std::wstring &wstr);
	static std::wstring str_to_wstr(const std::string &str);

	static std::string gbk_to_utf8(const std::string &str);
	static std::string utf8_to_gbk(const std::string &str);

    static std::size_t str_to_sign(const std::string &str);

    static std::string to_str(std::size_t size, const char* fmt, ...);

    /**
     * @brief高效string format函数
     * @ref: https://blog.csdn.net/u014417133/article/details/78309936
     *       http://blog.guorongfei.com/2016/03/10/python-like-str-formater-in-cpp/
     */
    template <typename ...Args>
    static std::string format(const std::string &fmt, const Args &...args);
    
    /**
     * @brief:
     * delimiter: char or std::string
     */
    template <typename Iter, typename Type>
    static std::string join(Iter begin, Iter end, Type delimiter);

    /**
     * @brief
     * 例如delimiter='/', str="a//b", 只有两个切片, a和b, 中间的会丢失
     */
    template <typename Type>
    static void split(const std::string &str, Type delimiter, std::vector<std::string> &res);

private:
    DISALLOW_COPY_AND_ASSIGN(StrUtil);

    template <typename Type, typename ...Args>
    static void _format_str_helper(
            std::ostringstream &oss,
            const std::string &fmt,
            const std::string::size_type &idx,
            const Type &next,
            const Args &...args);
    
    static void _format_str_helper(
            std::ostringstream &oss,
            const std::string &fmt,
            const std::string::size_type &idx);

private:
    StrUtil();
    ~StrUtil();
};

// -----------------------------------------------------------

template <typename Iter, typename Type>
std::string StrUtil::join(Iter begin, Iter end, Type delimiter) {
    std::ostringstream res;
    if (begin != end) {
        res << *begin++;
    }
    while (begin != end) {
        res << delimiter << *begin++;
    }
    return res.str();
}

template <typename Type>
void StrUtil::split(const std::string &str, Type delimiter, std::vector<std::string> &res) {
    if (str.empty()) {
        return;
    }
    if (!res.empty()) {
        res.clear();
    }
    std::size_t begin = 0;
    std::size_t end = str.find_first_of(delimiter, begin);
    while (end != std::string::npos) {
        res.push_back(str.substr(begin, end - begin));
        begin = str.find_first_not_of(delimiter, end);
        end = str.find_first_of(delimiter, begin);
    }
    if (begin != end) {
        res.push_back(str.substr(begin, end - begin));
    }
}

template <typename ...Args>
std::string StrUtil::format(const std::string &fmt, const Args &...args) {
    std::ostringstream oss;
    _format_str_helper(oss, fmt, 0, args...);
    return oss.str();
}

template <typename Type, typename ...Args>
void StrUtil::_format_str_helper(
        std::ostringstream &oss,
        const std::string &fmt,
        const std::string::size_type &idx,
        const Type &next,
        const Args &...args) {
    auto pos = fmt.find_first_of("{}", idx);
    if (pos == std::string::npos) {
        return _format_str_helper(oss, fmt, idx);
    }
    oss.write(fmt.data() + idx, pos - idx);
    oss << next;
    _format_str_helper(oss, fmt, pos + 2, args...);
}

}; // end of namespace tiny_engine
#endif // __STR_UTIL_H_

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
