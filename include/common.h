/*****************************************************************
*   Copyright (C) 2018. All rights reserved.
*
*   @file:    common.h
*   @author:  marinyoung@163.com
*   @date:    2018/11/22 00:22:59
*   @brief  
*
*****************************************************************/

#ifndef __COMMON_H_
#define __COMMON_H_

namespace tiny_engine {

//#define DEBUG

#ifndef DISALLOW_COPY_AND_ASSIGN
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
    TypeName(const TypeName &) = delete; \
    TypeName& operator=(const TypeName &) = delete;
#endif

#ifndef SAFE_DELETE_PTR
#define SAFE_DELETE_PTR(pointer) do { \
    if (nullptr != pointer) { \
        delete pointer; \
        pointer = nullptr; \
    } \
} while (0)
#endif

#ifndef SAFE_DELETE_ARR_PTR
#define SAFE_DELETE_ARR_PTR(pointer) do { \
    if (nullptr != pointer) { \
        delete[] pointer; \
        pointer = nullptr; \
    } \
} while (0)
#endif

#ifndef RETURN_ON_SUCC
#define RETURN_ON_SUCC true
#endif

#ifndef RETURN_ON_FAIL
#define RETURN_ON_FAIL false
#endif

#ifndef RETURN_ON_VOID
#define RETURN_ON_VOID
#endif

#ifndef AND
#define AND &&
#endif

#ifndef OR
#define OR ||
#endif

#ifndef XOR
#define XOR ^
#endif

#ifndef NOT
#define NOT !
#endif

#ifndef RANGE_DEFINATION
#define RANGE_DEFINATION
#define GT_LOWER_AND_LT_UPPER(value, lower, upper) (lower < value && value < upper)
#define GE_LOWER_AND_LT_UPPER(value, lower, upper) (lower <= value && value < upper)
#define GE_LOWER_AND_LE_UPPER(value, lower, upper) (lower <= value && value <= upper)
#define GT_LOWER_AND_LE_UPPER(value, lower, upper) (lower < value && value <= upper)
#endif

#ifndef CHECK_NULL
#define CHECK_NULL(pointer) (nullptr == (pointer))
#endif

// a == b
#ifndef EXPECT_EQ_OR_RETURN
#define EXPECT_EQ_OR_RETURN(actual_value, expected_value, return_value) do { \
    if ((expected_value) != (actual_value)) { \
        return return_value; \
    } \
} while (0)
#endif

// a != b
#ifndef EXPECT_NE_OR_RETURN
#define EXPECT_NE_OR_RETURN(actual_value, unexpected_value, return_value) do { \
    if ((unexpected_value) == (actual_value)) { \
        return return_value; \
    } \
} while (0)
#endif

// a > b
#ifndef EXPECT_GT_OR_RETURN
#define EXPECT_GT_OR_RETURN(bigger_value, smaller_value, return_value) do { \
    if ((bigger_value) <= (smaller_value)) { \
        return return_value; \
    } \
} while (0)
#endif

// a >= b
#ifndef EXPECT_GE_OR_RETURN
#define EXPECT_GE_OR_RETURN(bigger_value, smaller_value, return_value) do { \
    if ((bigger_value) < (smaller_value)) { \
        return return_value; \
    } \
} while (0)
#endif

// a < b
#ifndef EXPECT_LT_OR_RETURN
#define EXPECT_LT_OR_RETURN(smaller_value, bigger_value, return_value) do { \
    if ((smaller_value) >= (bigger_value)) { \
        return return_value; \
    } \
} while (0)
#endif

// a <= b
#ifndef EXPECT_LE_OR_RETURN
#define EXPECT_LE_OR_RETURN(smaller_value, bigger_value, return_value) do { \
    if ((smaller_value) > (bigger_value)) { \
        return return_value; \
    } \
} while (0)
#endif

// expect test(a) is true
#ifndef EXPECT_TRUE_OR_RETURN
#define EXPECT_TRUE_OR_RETURN(bool_expression, return_value_when_failed) do { \
    if (!(bool_expression)) { \
        return return_value_when_failed; \
    } \
} while (0)
#endif

// expect !test(a) is true
#ifndef EXPECT_FALSE_OR_RETURN
#define EXPECT_FALSE_OR_RETURN(bool_expression, return_value_when_truth) do { \
    if ((bool_expression)) { \
        return return_value_when_truth; \
    } \
} while (0)
#endif

}; // end of namespace tiny_engine

#endif // __COMMON_H_

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
