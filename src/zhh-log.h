#ifndef ZHH_LOG_H
#define ZHH_LOG_H

#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include "fmt/fmt/core.h"
#include <iostream>
#include <queue>
#include <chrono>
#include <thread>
#include <unordered_map>

struct LogItem
{
    int file_idx;
    std::string message;

    LogItem() : file_idx(0), message("") {}

    LogItem(int idx, std::string msg)
        : file_idx(idx), message(std::move(msg)) {}
};


class zhh_log
{
public:
    zhh_log();

    ~zhh_log();

    static zhh_log* instance();

    bool init(int cpu_core = -1, std::string path = "../data/");

    int open_file(std::string file_name);

    void destroy();

    template<typename... Args>
    void log_message( const int cur_idx, const std::string& level, const std::string& message, Args&&... args)
    {
        write_to_buffer(LogItem(cur_idx,
                        fmt::format("[{}]  {} \n",
                        level,
                        /*format_utc_time(get_utc_time(), true), */
                        fmt::format(message, std::forward<Args>(args)...))));
    }

    #define log_info(cur_idx, message, ...) zhh_log::instance()->log_message(cur_idx, "INFO", message, ##__VA_ARGS__)
    #define log_warn(cur_idx, message, ...) zhh_log::instance()->log_message(cur_idx, "WARN", message, ##__VA_ARGS__)
    #define log_error(cur_idx, message, ...) zhh_log::instance()->log_message(cur_idx, "ERROR", message, ##__VA_ARGS__)

    // Save original log, for storing the csv
    template<typename... Args>
    void log_original( const int cur_idx, const std::string& message, Args&&... args)
    {
        write_to_buffer(LogItem(cur_idx,
                        fmt::format("{}\n", fmt::format(message, std::forward<Args>(args)...))));
    }

    #define log_csv(cur_idx, message, ...) zhh_log::instance()->log_original(cur_idx, message, ##__VA_ARGS__)

    // 循环缓冲区操作（ring buffer setting）
    bool write_to_buffer(const LogItem& log);
    bool read_from_buffer(LogItem& log_read,
        std::chrono::milliseconds timeout = std::chrono::milliseconds(100));
    size_t get_valid_log_num() const;
    size_t get_remain_buffer_size() const;
    void clear_buffer();

    double get_utc_time();
    std::string format_utc_time(double t, bool with_millis = false);

private:
    void loop();
    bool bindCurrentThreadToCore(int cpu_core);


private:
    static zhh_log* s_instance;

    std::thread m_thread;
    std::atomic<bool> m_running{false};
    std::mutex m_mutex;
    std::condition_variable m_cv;
    int m_cpu_core = -1;

    std::string m_file_path;
    int m_file_idx = 1;
    std::unordered_map<int, FILE*> m_idx_file_map;

    // 循环缓冲区
    std::vector<LogItem> m_buffer;
    std::atomic<size_t> m_write_index{0};
    std::atomic<size_t> m_read_index{0};
    size_t m_buffer_size{10000};
};

#endif // ZHH_LOG_H
