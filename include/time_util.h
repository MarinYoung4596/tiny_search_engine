/*****************************************************************
*   Copyright (C) 2018 . All rights reserved.
*
*   @file:    time_util.h
*   @author:  marinyoung@163.com
*   @date:    2018/11/22 22:53:26
*   @brief:
*
*****************************************************************/

#ifndef __TIME_UTIL_H_
#define __TIME_UTIL_H_
#pragma once

#include "common.h"
#include <ctime>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <stdexcept>
#include <string>
#include <stdint.h>
#include <sys/time.h> // gettimeofday

namespace tiny_engine {

#define TIME_STD_FORMAT "%Y-%m-%d %H:%M:%S"

class TimeUtil {
public:
    using MAX_BUFF_SIZE = std::integral_constant<int, 256>;

    using ONE_WEEK_DAY = std::integral_constant<int, 7>;
    using ONE_WEEK_SECOND = std::integral_constant<int, 604800>;
    using ONE_DAY_HOUR = std::integral_constant<int, 24>;
    using ONE_DAY_MINUTE = std::integral_constant<int, 1440>;
    using ONE_DAY_SECOND = std::integral_constant<int, 86400>;
    using ONE_HOUR_SECOND = std::integral_constant<int, 3600>;
    using SIXTY = std::integral_constant<int, 60>;
    using ONE_THOUSAND = std::integral_constant<int, 1000>;
    using ONE_MILLION  = std::integral_constant<int, 1000000>;
    using ONE_BILLION  = std::integral_constant<int, 1000000000>;

    enum TIME_UNIT {
        NANOSECOND = 0/*纳秒, ns*/,
        MICROSECOND/*微秒, us*/,
        MILLISECOND/*毫秒, ms*/,
        SECOND,
        MINUTE,
        HOUR,
        DAY,
        WEEK
    };

    static uint32_t get_curr_timestamp();
    static std::string get_curr_format_time(
            const std::string &fmt = TIME_STD_FORMAT);

    static uint64_t time_convert(
            uint64_t input,
            TIME_UNIT in_type,
            TIME_UNIT out_type);

    /***************** tm (s) ****************/
    /**
    struct tm {
        int tm_sec;     // 秒 – 取值区间为[0,59]
        int tm_min;     // 分 - 取值区间为[0,59]
        int tm_hour;    // 时 - 取值区间为[0,23]
        int tm_mday;    // 一个月中的日期 - 取值区间为[1,31]
        int tm_mon;     // 月份（从一月开始，0代表一月） - 取值区间为[0,11]
        int tm_year;    // 年份，其值等于实际年份减去1900
        int tm_wday;    // 星期 – 取值区间为[0,6]，其中0代表星期天，1代表星期一，以此类推
        int tm_yday;    // 从每年的1月1日开始的天数 – 取值区间为[0,365]，
                            其中0代表1月1日，1代表1月2日，以此类推
        int tm_isdst;   // 夏令时标识符,
                            实行夏令时的时候，tm_isdst为正;
                            不实行夏令时的时候，tm_isdst为0;
                            不了解情况时，tm_isdst()为负
        long int tm_gmtoff; // 指定了日期变更线东面时区中UTC东部时区正秒数或UTC西部时区的负秒数
        const char *tm_zone; // 当前时区的名字(与环境变量TZ有关)
    };
    */
    static std::string timestamp_2_format_time(
            uint32_t timestamp,
            const std::string &fmt = TIME_STD_FORMAT);
    static uint32_t format_time_2_timestamp(
            const std::string &str,
            const std::string &fmt = TIME_STD_FORMAT);

    static uint32_t tm_2_timestamp(struct tm* ptm);
    static struct tm* timestamp_2_tm(uint32_t timestamp);

    static std::string tm_2_format_time(
            const struct tm* ptm,
            const std::string &fmt = TIME_STD_FORMAT);
    static bool format_time_2_tm(
            const std::string &str,
            const std::string &fmt,
            struct tm *tm);

    static struct tm* get_curr_tm();

    static double tm_diff_s(struct tm* begin, struct tm* end);

    /***************** timeval (us) ****************/
    /**
        struct timeval {
            time_t tv_sec;  // seconds
            long tv_usec;   // micro seconds
        };
    */
    static struct timeval get_curr_timeval();

    static uint64_t timeval_diff(
            const struct timeval* begin,
            const struct timeval* end,
            TIME_UNIT type);

    static uint64_t timeval_diff_us(const struct timeval* begin, const struct timeval* end);
    static uint64_t timeval_diff_ms(const struct timeval* begin, const struct timeval* end);
    static uint64_t timeval_diff_s(const struct timeval* begin, const struct timeval* end);

    /***************** timespec (us) ****************/
    /**
        struct timespec {
            time_t tv_sec;  // seconds ; >= 0
            long tv_nsec;   // nano seconds [0, 999999999]
        };
    */
    static struct timespec get_curr_timespec();

    static uint64_t timespec_diff(
            const struct timespec* begin,
            const struct timespec* end,
            TIME_UNIT type);

    static uint64_t timespec_diff_ns(const struct timespec* begin, const struct timespec* end);
    static uint64_t timespec_diff_us(const struct timespec* begin, const struct timespec* end);
    static uint64_t timespec_diff_ms(const struct timespec* begin, const struct timespec* end);
    static uint64_t timespec_diff_s(const struct timespec* begin, const struct timespec* end);

private:
    DISALLOW_COPY_AND_ASSIGN(TimeUtil);
    static uint64_t _time_convert_helper(TIME_UNIT type);

private:
    TimeUtil();
    ~TimeUtil();
};

}; // end of namespace tiny_engine
#endif // __TIME_UTIL_H_

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
