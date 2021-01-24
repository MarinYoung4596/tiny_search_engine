/*****************************************************************
*   Copyright (C) 2018 . All rights reserved.
*
*   @file:    main.cpp
*   @author:  marinyoung@163.com
*   @date:    2018/11/21 22:54:03
*   @brief:   
*
*****************************************************************/

#include "../include/common.h"
#include "../include/table.h"

#include <iostream>
#include <unistd.h>
#include <getopt.h>

using namespace tiny_engine;

struct globalOptions {
    int mode;
    std::string conf_path;
    std::string index_path;
} g_options;

void parse_args(int argc, char **argv) {
    char ch;
    while ((ch = getopt(argc, argv, "x:c:d:")) != EOF) { // ×¢Òâ×Ö·û´®Ä©Î²µÄ":"
        switch (ch) {
            case 'x':
                g_options.index_path = optarg;
                break;
            case 'c':
                g_options.conf_path = optarg;
                break;
            case 'd':
                try {
                    g_options.mode = std::stoi(std::string(optarg));
                } catch (std::exception &e) {
                    LOG_ERROR("convert optarg to int error, raw[%s]", optarg);
                }
                break;
            default:
                break;
        }
    }
}

int main(int argc, char **argv) {
    // parse args
    parse_args(argc, argv);

#ifdef DEBUG
    auto time_1st = TimeUtil::get_curr_timeval();
#endif

    // init log

    // init search engine
    std::unique_ptr<TinyEngine> engine = nullptr;
    if (g_options.conf_path.empty()) {
        engine.reset(new TinyEngine());
    } else {
        engine.reset(new TinyEngine(g_options.conf_path));
    }
    EXPECT_TRUE_OR_RETURN(engine->init(), -1);

#ifdef DEBUG
    auto time_2nd = TimeUtil::get_curr_timeval();
#endif

    auto num = 0;
    std::string query;
    std::vector<std::pair<std::string, std::string>> result; // [<title, url> ...]
    for (; std::getline(std::cin, query); ) {
        if (query == "exit") {
            break;
        }
    	if (!engine->search(query, result)) {
    		continue;
    	}
        ++num;

        if (g_options.mode == 0) {
            continue;
        }
    	for (auto it = result.begin(); it != result.end(); ++it) {
    		std::cout << query << '\t' << it->first << '\t' << it->second << '\n';
    	}
    }

#ifdef DEBUG
    auto time_3rd = TimeUtil::get_curr_timeval();
    auto init_time = TimeUtil::timeval_diff_ms(&time_2nd, &time_1st);
    auto search_time = TimeUtil::timeval_diff_ms(&time_3rd, &time_2nd);
    LOG_INFO("init_time[%lu ms] query[%d] search_time[%lu ms] avg_tm[%lu ms] qps[%lu]",
            init_time, num, search_time, search_time/num, num/(search_time/1000));
#endif

    // stop
    EXPECT_TRUE_OR_RETURN(engine->stop(), -1);

    return 0;
}
/* vim: set expandtab ts=4 sw=4 sts=4 tw=100: */
