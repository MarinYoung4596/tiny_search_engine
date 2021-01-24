/*****************************************************************
*   Copyright (C) 2018 . All rights reserved.
*   
*   @file:    file_util.cpp
*   @author:  marinyoung@163.com
*   @date:    2018/12/27 22:51:43
*   @brief:   
*
*****************************************************************/


#include "../include/file_util.h"

namespace tiny_engine {

bool FileUtil::delete_file(const std::string &file_path) {
    return 0 == remove(file_path.c_str());
}

bool FileUtil::delete_files(const std::vector<std::string> &files) {
    for (auto it = files.begin(); it != files.end(); ++it) {
        if (!delete_file(*it)) {
            return false;
        }
    }
    return true;
}

bool FileUtil::get_prefix_files(
        const std::string &dir,
        const std::string &prefix,
        std::vector<std::string> &files) {
    EXPECT_FALSE_OR_RETURN(dir.empty(), false);
    EXPECT_FALSE_OR_RETURN(prefix.empty(), false);
    if (!files.empty()) {
        files.clear();
    }
    std::vector<std::string> all_dir_files;
    EXPECT_TRUE_OR_RETURN(get_dir_files(dir, all_dir_files), false);
    std::vector<std::string> vec;
    for (const auto &item : all_dir_files) {
        StrUtil::split(item, '/', vec);
        if (vec.empty()) {
            continue;
        }
        auto &file_name = vec[vec.size() - 1];
        if (StrUtil::is_start_with(file_name, prefix)) {
            files.push_back(item);
        }
    }
    return true;
}

bool FileUtil::get_postfix_files(
		const std::string &dir, 
		const std::string &postfix, 
		std::vector<std::string> &files) {
    EXPECT_FALSE_OR_RETURN(dir.empty(), false);
    EXPECT_FALSE_OR_RETURN(postfix.empty(), false);
    if (!files.empty()) {
        files.clear();
    }
    std::vector<std::string> all_dir_files;
    EXPECT_TRUE_OR_RETURN(get_dir_files(dir, all_dir_files), false);
    std::vector<std::string> vec;
    for (const auto &item : all_dir_files) {
        StrUtil::split(item, '/', vec);
        if (vec.empty()) {
            continue;
        }
        auto &file_name = vec[vec.size() - 1];
        if (StrUtil::is_end_with(file_name, postfix)) {
            files.push_back(item);
        }
    }
    return true;
}

bool FileUtil::get_dir_files(const std::string &dir, std::vector<std::string> &files) {
    EXPECT_FALSE_OR_RETURN(dir.empty(), false);
    if (!files.empty()) {
        files.clear();
    }
    DIR* dp = opendir(dir.c_str());
    EXPECT_NE_OR_RETURN(nullptr, dp, false);
    while (true) {
        struct dirent* entry = readdir(dp);
        if (CHECK_NULL(entry)) {
            break;
        }
        if (entry->d_type != DT_REG) {
            continue;
        }
        files.push_back(dir + "/" + entry->d_name);
    }
    closedir(dp);
    return true;
}

bool FileUtil::is_exist(const std::string &file_path) {
    return 0 == ::access(file_path.c_str(), F_OK);
}

bool FileUtil::is_readable(const std::string &file_path) {
    return 0 == ::access(file_path.c_str(), F_OK | R_OK);
}

bool FileUtil::is_writable(const std::string &file_path) {
    return 0 == ::access(file_path.c_str(), F_OK | W_OK);
}

bool FileUtil::rename(const std::string &old_name, const std::string &new_name) {
    return 0 == ::rename(old_name.c_str(), new_name.c_str());
}

bool FileUtil::generate_md5_file(const std::string &file_path) {
    EXPECT_FALSE_OR_RETURN(file_path.empty(), false);
    std::string cmd = "md5sum " + file_path;
    std::string output;
    if (!run_command(cmd.c_str(), &output)) {
        // log
        return false;
    }
    std::string md5_file = file_path + ".md5";
    std::ofstream ofs(md5_file, std::ofstream::out);
    if (!ofs.is_open()) {
        // log
        return false;
    }
    ofs.write(output.c_str(), output.size());
    return true;
}

bool FileUtil::check_md5(const std::string &file_path) {
    EXPECT_TRUE_OR_RETURN(is_readable(file_path), false);
    auto real_md5 = get_md5(file_path);
    
    std::string md5_file = file_path + ".md5";
    EXPECT_TRUE_OR_RETURN(is_readable(md5_file), false);
    std::ifstream ifs(md5_file.c_str(), std::ifstream::in);
    EXPECT_TRUE_OR_RETURN(ifs.is_open(), false);
    std::string line;
    std::getline(ifs, line);
    auto pos = line.find_first_of(' ');
    EXPECT_NE_OR_RETURN(std::string::npos, pos, false);
    auto logged_md5 = line.substr(0, pos);
    
    return real_md5 == logged_md5;
}

std::string FileUtil::get_md5(const std::string &file_path) {
    EXPECT_FALSE_OR_RETURN(file_path.empty(), std::string());
    std::string cmd = StrUtil::format("md5sum {} | awk '{print $1}'", file_path);
    std::string result;
    if (!run_command(cmd.c_str(), &result)) {
        // log
        return std::string();
    }
    return result;
}

bool FileUtil::run_command(const std::string &cmd, std::string* out_msg) {
    EXPECT_FALSE_OR_RETURN(cmd.empty(), false);
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (CHECK_NULL(pipe)) {
        if (nullptr != out_msg) {
            *out_msg = StrUtil::format("popen failed, cmd[%s]", cmd);
        }
        return false;
    }
    std::array<char, 128> buf;
    while (fgets(buf.data(), buf.size(), pipe.get()) != nullptr) {
        if (nullptr != out_msg) {
            out_msg->append(buf.data());
        }
    }
    return true;
}

}; // end of namespace tiny_engine
/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
