/*****************************************************************
*   Copyright (C) 2018 . All rights reserved.
*
*   @file:    time_util.cpp
*   @author:  marinyoung@163.com
*   @date:    2018/12/27 22:51:53
*   @brief:   
*
*****************************************************************/

#include "../include/time_util.h"

namespace tiny_engine {

uint32_t TimeUtil::get_curr_timestamp() {
    time_t t = time(nullptr);
    return static_cast<uint32_t>(t);
}

std::string TimeUtil::get_curr_format_time(const std::string &fmt) {
    time_t curr_time = time(nullptr);
    char fmt_time[MAX_BUFF_SIZE::value];
    strftime(fmt_time, sizeof(fmt_time), fmt.c_str(), localtime(&curr_time));
    return std::string(fmt_time);
}

uint64_t TimeUtil::time_convert(
        uint64_t input,
        TIME_UNIT in_type,
        TIME_UNIT out_type) {
    if (in_type == out_type) {
        return input;
    }
    // trans to value in ns
    uint64_t multiplier = _time_convert_helper(in_type); 
    uint64_t divisor = _time_convert_helper(out_type);
    auto ret = input;
    try {
        ret = input * multiplier / divisor;
    } catch (const std::exception &e) {
        throw std::out_of_range("value out of range!");
    }
    return ret;
}

uint64_t TimeUtil::_time_convert_helper(TIME_UNIT type) {
    uint64_t ret = 1;
    switch (type) {
        // case WEEK:
        //     ret = ONE_WEEK_SECOND::value * ONE_BILLION::value;
        //     break;
        // case DAY:
        //     ret = ONE_DAY_SECOND::value * ONE_BILLION::value;
        //     break;
        // case HOUR:
        //     ret = ONE_HOUR_SECOND::value * ONE_BILLION::value;
        //     break;
        // case MINUTE:
        //     ret = SIXTY::value * ONE_BILLION::value;
        //     break;
        case SECOND:
            ret = ONE_BILLION::value;
            break;
        case MILLISECOND:
            ret = ONE_MILLION::value;
            break;
        case MICROSECOND:
            ret = ONE_THOUSAND::value;
            break;
        case NANOSECOND:
            ret = 1;
            break;
        default:
            throw std::out_of_range("type out of range!");
            break;
    }
    return ret;
}

/***************** tm (s) ****************/
std::string TimeUtil::timestamp_2_format_time(
        uint32_t timestamp,
        const std::string &fmt) {
    struct tm* ptm = timestamp_2_tm(timestamp);
    return tm_2_format_time(ptm, fmt);
}

uint32_t TimeUtil::format_time_2_timestamp(
        const std::string &str,
        const std::string &fmt) {
    struct tm t;
    if (!format_time_2_tm(str, fmt, &t)) {
        throw std::logic_error("input not match!");
    }
    return tm_2_timestamp(&t);
}

struct tm* TimeUtil::timestamp_2_tm(uint32_t timestamp) {
    time_t t = static_cast<time_t>(timestamp); // typedef __int64 time_t;
    struct tm* ptm = localtime(&t);
    return ptm;
}

uint32_t TimeUtil::tm_2_timestamp(struct tm* ptm) {
    time_t t = mktime(ptm);
    return static_cast<uint32_t>(t);
}

std::string TimeUtil::tm_2_format_time(
        const struct tm* ptm,
        const std::string &fmt) {
    char buf[MAX_BUFF_SIZE::value];
    strftime(buf, MAX_BUFF_SIZE::value, fmt.c_str(), ptm);
    return std::string(buf);
}

bool TimeUtil::format_time_2_tm(
        const std::string &str,
        const std::string &fmt,
        struct tm* tm) {
    if (nullptr != strptime(str.c_str(), fmt.c_str(), tm)) {
        return true;
    }
    return false;
}

struct tm* TimeUtil::get_curr_tm() {
    time_t t = time(nullptr);
    struct tm *p = localtime(&t);
    return p;
}

double TimeUtil::tm_diff_s(struct tm* begin, struct tm* end) {
    time_t t_begin = mktime(begin);
    time_t t_end = mktime(end);
    return difftime(t_begin, t_end);
}

/***************** timeval (us) ****************/
struct timeval TimeUtil::get_curr_timeval() {
    struct timeval curr_time;
    gettimeofday(&curr_time, nullptr);
    return curr_time;
}

uint64_t TimeUtil::timeval_diff(
        const struct timeval* begin,
        const struct timeval* end,
        TIME_UNIT type) {
    auto delta_s = end->tv_sec - begin->tv_sec;
    auto delta_us = end->tv_usec - begin->tv_usec;
    auto diff = abs(delta_s * ONE_MILLION::value + delta_us);

    return time_convert(diff, MICROSECOND, type);
}

uint64_t TimeUtil::timeval_diff_us(
        const struct timeval* begin,
        const struct timeval* end) {
    return timeval_diff(begin, end, MICROSECOND);
}

uint64_t TimeUtil::timeval_diff_ms(
        const struct timeval* begin,
        const struct timeval* end) {
    return timeval_diff(begin, end, MILLISECOND);
}

uint64_t TimeUtil::timeval_diff_s(
        const struct timeval* begin,
        const struct timeval* end) {
    return timeval_diff(begin, end, SECOND);
}

/***************** timespec (ns) ****************/
struct timespec TimeUtil::get_curr_timespec() {
    struct timespec curr_time;
    clock_gettime(CLOCK_REALTIME, &curr_time);
    //std::timespec_get(&curr_time, TIME_UTC);
    return curr_time;
}

uint64_t TimeUtil::timespec_diff(
        const struct timespec* begin,
        const struct timespec* end,
        TIME_UNIT type) {
    auto delta_s = end->tv_sec - begin->tv_sec;
    auto delta_ns = end->tv_nsec - begin->tv_nsec;
    auto diff = abs(delta_s * ONE_BILLION::value + delta_ns);

    return time_convert(diff, NANOSECOND, type);
}

uint64_t TimeUtil::timespec_diff_ns(
        const struct timespec* begin,
        const struct timespec* end) {
    return timespec_diff(begin, end, NANOSECOND);
}

uint64_t TimeUtil::timespec_diff_us(
        const struct timespec* begin,
        const struct timespec* end) {
    return timespec_diff(begin, end, MICROSECOND);
}

uint64_t TimeUtil::timespec_diff_ms(
        const struct timespec* begin,
        const struct timespec* end) {
    return timespec_diff(begin, end, MILLISECOND);
}

uint64_t TimeUtil::timespec_diff_s(
        const struct timespec* begin,
        const struct timespec* end) {
    return timespec_diff(begin, end, SECOND);
}

}; // end of namespace tiny_engine
/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
