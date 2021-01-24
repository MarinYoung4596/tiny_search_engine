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
        NANOSECOND = 0/*����, ns*/,
        MICROSECOND/*΢��, us*/,
        MILLISECOND/*����, ms*/,
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
        int tm_sec;     // �� �C ȡֵ����Ϊ[0,59]
        int tm_min;     // �� - ȡֵ����Ϊ[0,59]
        int tm_hour;    // ʱ - ȡֵ����Ϊ[0,23]
        int tm_mday;    // һ�����е����� - ȡֵ����Ϊ[1,31]
        int tm_mon;     // �·ݣ���һ�¿�ʼ��0����һ�£� - ȡֵ����Ϊ[0,11]
        int tm_year;    // ��ݣ���ֵ����ʵ����ݼ�ȥ1900
        int tm_wday;    // ���� �C ȡֵ����Ϊ[0,6]������0���������죬1��������һ���Դ�����
        int tm_yday;    // ��ÿ���1��1�տ�ʼ������ �C ȡֵ����Ϊ[0,365]��
                            ����0����1��1�գ�1����1��2�գ��Դ�����
        int tm_isdst;   // ����ʱ��ʶ��,
                            ʵ������ʱ��ʱ��tm_isdstΪ��;
                            ��ʵ������ʱ��ʱ��tm_isdstΪ0;
                            ���˽����ʱ��tm_isdst()Ϊ��
        long int tm_gmtoff; // ָ�������ڱ���߶���ʱ����UTC����ʱ����������UTC����ʱ���ĸ�����
        const char *tm_zone; // ��ǰʱ��������(�뻷������TZ�й�)
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
