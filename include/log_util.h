/*****************************************************************
*   Copyright (C) 2018 . All rights reserved.
*   
*   @file:    log_util.h
*   @author:  marinyoung@163.com
*   @date:    2018/12/19 19:52:27
*   @brief:   
*
*****************************************************************/

#ifndef __LOG_UTIL_H_
#define __LOG_UTIL_H_

#include "time_util.h"
#include <iostream>
#include <cstdio>
#include <fstream>
#include <memory>

namespace tiny_engine {

#define DEBUG_LEVEL   (0b0001)
#define INFO_LEVEL    (0b0010)
#define WARNING_LEVEL (0b0100)
#define ERROR_LEVEL   (0b1000)

#define DEBUG_STR   ("DEBUG")
#define INFO_STR    ("INFO")
#define WARNING_STR ("WARNING")
#define ERROR_STR   ("FATAL")

#define COLOR_NONE      ("\033[0m")
#define COLOR_RED       ("\033[0;31m")
#define COLOR_RED_LIGHT ("\033[1;31m")
#define COLOR_GREEN     ("\033[0;32m")
#define COLOR_BLUE      ("\033[0;34m")


#define _LOG_(log_level, log_flag, color, format, arg...) do { \
    std::string stime = TimeUtil::get_curr_format_time("%m-%d %H:%M:%S"); \
    if (log_level < WARNING_LEVEL) { \
        fprintf(stdout, "%s%s%s: %s %s:%d %s()] " format "\n", \
            color, log_flag, COLOR_NONE, \
            stime.c_str(), __FILE__, __LINE__, __FUNCTION__, ## arg); \
    } else { \
        fprintf(stderr, "%s%s%s: %s %s:%d %s()] " format "\n", \
            color, log_flag, COLOR_NONE, \
            stime.c_str(), __FILE__, __LINE__, __FUNCTION__, ## arg); \
    } \
} while (0)


#ifndef LOG_DEBUG
#define LOG_DEBUG(format, arg...)   _LOG_(DEBUG_LEVEL, DEBUG_STR, COLOR_NONE, format, ## arg)
#endif

#ifndef LOG_INFO
#define LOG_INFO(format, arg...)    _LOG_(INFO_LEVEL, INFO_STR, COLOR_GREEN, format, ## arg)
#endif

#ifndef LOG_WARNING
#define LOG_WARNING(format, arg...) _LOG_(WARNING_LEVEL, WARNING_STR, COLOR_BLUE, format, ## arg)
#endif

#ifndef LOG_ERROR
#define LOG_ERROR(format, arg...)   _LOG_(ERROR_LEVEL, ERROR_STR, COLOR_RED, format, ## arg)
#endif

#ifndef EXPECT_EQ_OR_RETURN_LOGGED
#define EXPECT_EQ_OR_RETURN_LOGGED(actual_value, expected_value, return_value, log_str) do { \
    if ((expected_value) != (actual_value)) { \
        LOG_INFO(log_str); \
        return return_value; \
    } \
} while (0)
#endif

#ifndef EXPECT_NE_OR_RETURN_LOGGED
#define EXPECT_NE_OR_RETURN_LOGGED(actual_value, unexpected_value, return_value, log_str) do { \
    if ((unexpected_value) == (actual_value)) { \
        LOG_INFO(log_str); \
        return return_value; \
    } \
} while (0)
#endif

#ifndef EXPECT_GT_OR_RETURN_LOGGED
#define EXPECT_GT_OR_RETURN_LOGGED(bigger_value, smaller_value, return_value, log_str) do { \
    if ((bigger_value) <= (smaller_value)) { \
        LOG_INFO(log_str); \
        return return_value; \
    } \
} while (0)
#endif

#ifndef EXPECT_GE_OR_RETURN_LOGGED
#define EXPECT_GE_OR_RETURN_LOGGED(bigger_value, smaller_value, return_value, log_str) do { \
    if ((bigger_value) < (smaller_value)) { \
        LOG_INFO(log_str); \
        return return_value; \
    } \
} while (0)
#endif

#ifndef EXPECT_LT_OR_RETURN_LOGGED
#define EXPECT_LT_OR_RETURN_LOGGED(smaller_value, bigger_value, return_value, log_str) do { \
    if ((smaller_value) >= (bigger_value)) { \
        LOG_INFO(log_str); \
        return return_value; \
    } \
} while (0)
#endif

#ifndef EXPECT_LE_OR_RETURN_LOGGED
#define EXPECT_LE_OR_RETURN_LOGGED(smaller_value, bigger_value, return_value, log_str) do { \
    if ((smaller_value) > (bigger_value)) { \
        LOG_INFO(log_str); \
        return return_value; \
    } \
} while (0)
#endif

#ifndef EXPECT_TRUE_OR_RETURN_LOGGED
#define EXPECT_TRUE_OR_RETURN_LOGGED(bool_expression, return_value_when_failed, log_str) do { \
    if (!(bool_expression)) { \
        LOG_INFO(log_str); \
        return return_value_when_failed; \
    } \
} while (0)
#endif

#ifndef EXPECT_FALSE_OR_RETURN_LOGGED
#define EXPECT_FALSE_OR_RETURN_LOGGED(bool_expression, return_value_when_truth, log_str) do { \
    if ((bool_expression)) { \
        LOG_INFO(log_str); \
        return return_value_when_truth; \
    } \
} while (0)
#endif


}; // end of namespace tiny_engine

#endif // __LOG_UTIL_H_

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
