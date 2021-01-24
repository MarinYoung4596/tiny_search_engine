/*****************************************************************
*   Copyright (C) 2018 . All rights reserved.
*   
*   @file:    file_util.h
*   @author:  marinyoung@163.com
*   @date:    2018/12/24 18:49:38
*   @brief:   
*
*****************************************************************/


#ifndef __FILE_UTIL_H_
#define __FILE_UTIL_H_

#include "common.h"
#include "str_util.h"
#include <vector>
#include <array>
#include <string>
#include <cstring>
#include <memory> // unique_ptr
#include <fstream>
#include <unistd.h> // F_OK, R_OK, W_OK
#include <dirent.h>
#include <errno.h>

namespace tiny_engine {

class FileUtil {
public:
    static bool delete_file(const std::string &file_path);
    static bool delete_files(const std::vector<std::string> &files);
    
    static bool get_dir_files(const std::string &dir, std::vector<std::string> &files);
    static bool get_prefix_files(
            const std::string &dir,
            const std::string &prefix,
            std::vector<std::string> &files);
    static bool get_postfix_files(
            const std::string &dir,
            const std::string &postfix,
            std::vector<std::string> &files);
    
    static bool is_exist(const std::string &file_path);
    static bool is_readable(const std::string &file_path);
    static bool is_writable(const std::string &file_path);
    
    static bool rename(const std::string &old_name, const std::string &new_name);
    
    static bool generate_md5_file(const std::string &file_path);
    static bool check_md5(const std::string &file_path);
    static std::string get_md5(const std::string &file_path);

    static bool run_command(const std::string &cmd, std::string *out_msg = nullptr);

private:
    DISALLOW_COPY_AND_ASSIGN(FileUtil);

private:
    FileUtil();
    ~FileUtil();
    
}; // end of class FileUtil

}; // end of namespace tiny_engine

#endif // __FILE_UTIL_H

/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
