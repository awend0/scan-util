#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <stack>
#include <string>
#include <filesystem>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <set>
#include <map>
#include <algorithm>
#include "mingw.thread.h"
#include "mingw.mutex.h"

#define IOMANIP_MOD std::setfill('0') << std::setw(2) <<

struct multiset_comparator
{
    bool operator()(const std::shared_ptr<std::filesystem::directory_entry> &lhs, const std::shared_ptr<std::filesystem::directory_entry> &rhs) const {
        return std::filesystem::file_size(*lhs) > std::filesystem::file_size(*rhs);
    }
};

class Scanner
{
    public:
        Scanner(std::string path, size_t threads_count, std::pair<std::unordered_map<std::string, std::vector<std::string>>, std::vector<std::vector<std::string>>> sig_pairs);
        void scan(void);

    private:
        void routine(void);
        void check_file(std::wstring filename, std::string ext);
        void print_result(void);
        void set_timestamp(void);

        std::string _path;
        const size_t _threads_count;
        std::unordered_map<std::string, std::vector<std::string>> _signatures;
        std::vector<std::vector<std::string>> _extensions;

        std::mutex multiset_pop_mutex, counter_inc_mutex;
        std::unordered_map<std::string, int> _detects;
        std::chrono::high_resolution_clock::time_point _start_timepoint;
        std::multiset<std::shared_ptr<std::filesystem::directory_entry>, multiset_comparator> _files;
        std::atomic<int> _errors, _processed;
        std::string _timestamp;
};
