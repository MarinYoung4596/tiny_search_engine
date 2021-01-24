/*****************************************************************
*   Copyright (C) 2018 . All rights reserved.
*   
*   @file:    str_util.cpp
*   @author:  marinyoung@163.com
*   @date:    2018/12/27 22:52:00
*   @brief  
*
*****************************************************************/


#include "../include/str_util.h"

namespace tiny_engine {

std::string StrUtil::left_trim(const std::string &str) {
    if (str.empty()) {
        return str;
    }
	std::size_t head = 0;
	for (; head < str.size() && std::isspace(str[head]); ++head) {}
	return head < str.size() ? \
		str.substr(head, str.size() - head + 1) : \
		std::string();
}

std::string StrUtil::right_trim(const std::string &str) {
    if (str.empty()) {
        return str;
    }
	std::size_t tail = str.size() - 1;
	for (; tail >= 0 && std::isspace(str[tail]); --tail) {}
	return tail < 0 ? \
		std::string() : \
		str.substr(0, tail + 1);
}

std::string StrUtil::trim(const std::string &str) {
    if (str.empty()) {
        return str;
    }
    std::size_t head = 0;
    for (; head < str.size() && std::isspace(str[head]); ++head) {} 
    std::size_t tail = str.size() - 1;
    for (; tail >= head && std::isspace(str[tail]); --tail) {} 
	return head > tail ? \
		std::string() : \
		str.substr(head, tail - head + 1);
}

std::string StrUtil::left_trim_with(const std::string &str, char ch) {
    if (str.empty()) {
        return str;
    }
    std::size_t head = 0;
	for (; head < str.size() && (str[head] == ch); ++head) {}
	return head < str.size() ? \
		str.substr(head, str.size() - head + 1) : \
		std::string();
}

std::string StrUtil::right_trim_with(const std::string &str, char ch) {
    if (str.empty()) {
        return str;
    }
    std::size_t tail = str.size() - 1;
	for (; tail >= 0 && (str[tail] == ch); --tail) {}
	return tail < 0 ? \
		std::string() : \
		str.substr(0, tail + 1);
}

std::string StrUtil::trim_with(const std::string &str, char ch) {
    if (str.empty()) {
        return str;
    }
    std::size_t head = 0;
    for (; head < str.size() && (str[head] == ch); ++head) {} 
    std::size_t tail = str.size() - 1;
    for (; tail >= head && (str[tail] == ch); --tail) {} 
	return head > tail ? \
		std::string() : \
		str.substr(head, tail - head + 1);
}

std::string StrUtil::wstr_to_str(const std::wstring &wstr) {
	/*
	using F = std::codecvt_byname<wchar_t, char, std::mbstate_t>;
	std::wstring_convert<F> conv(new F("CHS"));
	return conv.to_bytes(wstr);
	*/
    // TODO
    return std::string();
}

std::wstring StrUtil::str_to_wstr(const std::string &str) {
	/*
	using F = std::codecvt_byname<wchar_t, char, std::mbstate_t>;
	std::wstring_convert<F> conv(new F("CHS"));
	return conv.from_bytes(str);
	*/
    // TODO
    return std::wstring();
}

std::string StrUtil::utf8_to_gbk(const std::string &str) {
	// TODO
    return std::string();
}

std::string StrUtil::gbk_to_utf8(const std::string &str) {
    // TODO
    return std::string();
}

std::size_t StrUtil::str_to_sign(const std::string &str) {
    std::hash<std::string> h;
    return h(str);
}

std::string StrUtil::to_lower_case(const std::string &str) {
	std::string result(str.size(), '\0');
	std::transform(str.begin(), str.end(), result.begin(), ::tolower);
    return result;
}

std::string StrUtil::to_upper_case(const std::string &str) {
    std::string result(str.size(), '\0');
    std::transform(str.begin(), str.end(), result.begin(), ::toupper);
    return result;
}

bool StrUtil::is_digital_str(const std::string &str) {
    for (const auto &ch : str) {
        if (!std::isdigit(ch)) {
            return false;
        }
    }
    return true;
}

bool StrUtil::is_alpha_str(const std::string &str) {
    for (const auto &ch : str) {
        if (!std::isalpha(ch)) {
            return false;
        }
    }
    return true;
}

bool StrUtil::is_alnum_str(const std::string &str) {
    for (const auto &ch : str) {
        if (!std::isalnum(ch)) {
            return false;
        }
    }
    return true;
}

bool StrUtil::is_space_str(const std::string &str) {
    for (const auto &ch : str) {
        if (!std::isspace(ch & 0xff)) {
            return false;
        }
    }
    return true;
}

bool StrUtil::is_en_str(const std::string &str) {
    for (const auto &ch : str) {
        if (!std::isalnum(ch) && !std::ispunct(ch)) {
            return false;
        }
    }
    return true;
}

bool StrUtil::is_zh_str(const std::string &str) {
    // TODO
    return true;
}

bool StrUtil::is_en_punct_str(const std::string &str) {
    for (const auto &ch : str) {
        if (!std::ispunct(ch)) {
            return false;
        }
    }
    return true;
}

bool StrUtil::is_zh_punct_str(const std::string &str) {
    // TODO
    return true;
}

bool StrUtil::is_lower_str(const std::string &str) {
    for (const auto &ch : str) {
        if (std::isupper(ch)) {
            return false;
        }
    }
    return true;
}

bool StrUtil::is_upper_str(const std::string &str) {
    for (auto &ch : str) {
        if (std::islower(ch)) {
            return false;
        }
    }
    return true;
}

bool StrUtil::is_start_with(const std::string &str, const std::string &prefix) {
    return str.size() >= prefix.size()
		&& std::equal(prefix.cbegin(), prefix.cend(), str.cbegin());
}

bool StrUtil::is_end_with(const std::string &str, const std::string &postfix) {
    return str.size() >= postfix.size()
		&& std::equal(postfix.crbegin(), postfix.crend(), str.crbegin());
}

std::string StrUtil::to_str(std::size_t size, const char* fmt, ...) {
    char* buf = new char[size];
    memset(buf, '\0', size);
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, size, fmt, args);
    va_end(args);
    std::string result(buf);
    SAFE_DELETE_ARR_PTR(buf);
    return result;
}

void StrUtil::_format_str_helper(
        std::ostringstream &oss,
        const std::string &fmt,
        const std::string::size_type &idx) {
    auto ch_left = fmt.size() - idx;
    if (ch_left > 0) {
        oss.write(fmt.data() + idx, ch_left);
    }
}

}; // end of namespace tiny_engine
/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
