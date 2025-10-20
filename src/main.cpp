#include "zhh-log.h"
#include <ctime>
#include <thread>
#include <atomic>

// 全局停止标志
std::atomic<bool> g_running{true};

// 输入源1的线程函数
void test_1(zhh_log* log, int file_idx)
{
    static int counter = 0;
    while (g_running.load())
    {
        for(int j = 0; j < 50; j++)
        {
            // 测试不同类型的日志消息
            switch (j % 8) {
                case 0:
                    log_info(file_idx,
                        "[SOURCE-1] Basic types - int:{}, float:{}, double:{}, string:{} -- {}",
                        counter, 3.14f, 2.718281828, "Hello, World!", log->format_utc_time(log->get_utc_time(), true));
                    break;
                case 1:
                    log_info(file_idx,
                        "[SOURCE-1] Long message test - This is a very long log message that contains multiple sentences and should test the buffer capacity and formatting capabilities of the logging system. Counter: {}, Time: {}",
                        counter, log->format_utc_time(log->get_utc_time(), true));
                    break;
                case 2:
                    log_info(file_idx,
                        "[SOURCE-1] JSON-like data - {{\"id\":{}, \"status\":\"active\", \"value\":{:.6f}, \"timestamp\":\"{}\"}}",
                        counter, 3.14159265359, log->format_utc_time(log->get_utc_time(), true));
                    break;
                case 3:
                    log_info(file_idx,
                        "[SOURCE-1] Array data - [{}][{}][{}][{}][{}] - Counter: {}",
                        counter%10, (counter+1)%10, (counter+2)%10, (counter+3)%10, (counter+4)%10, counter);
                    break;
                case 4:
                    log_info(file_idx,
                        "[SOURCE-1] Special chars - Unicode: 中文测试, Symbols: !@#$%^&*(), Math: pi={:.6f}, e={:.6f}",
                        3.14159265359, 2.71828182846);
                    break;
                case 5:
                    log_info(file_idx,
                        "[SOURCE-1] Performance test - Processing item {} of {} with efficiency {:.2f}% at {}",
                        counter, 10000, (counter % 100) * 1.0, log->format_utc_time(log->get_utc_time(), true));
                    break;
                case 6:
                    log_info(file_idx,
                        "[SOURCE-1] Network data - IP:192.168.1.{}, Port:{}, Bytes:{}, Latency:{:.3f}ms",
                        counter%255, 8080 + (counter%1000), counter*1024, (counter%100)*0.1);
                    break;
                case 7:
                    log_info(file_idx,
                        "[SOURCE-1] System status - CPU:{:.1f}%, Memory:{:.1f}%, Disk:{:.1f}%, Network:{:.1f}% - Iteration: {}",
                        (counter%100)*0.1, (counter%90)*0.1, (counter%80)*0.1, (counter%70)*0.1, counter);
                    break;
            }
            counter++;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
    }
}

// 输入源2的线程函数
void test_2(zhh_log* log, int file_idx)
{
    static int counter = 0;
    while (g_running.load())
    {
        for(int j = 0; j < 80; j++)
        {
            // 测试不同类型的警告消息
            switch (j % 10) {
                case 0:
                    log_warn(file_idx, "[SOURCE-2] Simple warning - Counter: {}, Time: {}",
                        counter, log->format_utc_time(log->get_utc_time(), true));
                    break;
                case 1:
                    log_warn(file_idx, "[SOURCE-2] Memory warning - Available memory: {:.2f}GB, Used: {:.2f}GB, Threshold: {:.2f}GB",
                        (1000 - counter%1000)*0.001, (counter%1000)*0.001, 0.5);
                    break;
                case 2:
                    log_warn(file_idx, "[SOURCE-2] Network warning - Connection timeout after {}ms, Retry attempt: {}, Server: 192.168.1.{}:{}",
                        (counter%30)*100, counter%5, counter%255, 8080 + (counter%100));
                    break;
                case 3:
                    log_warn(file_idx, "[SOURCE-2] Performance warning - Response time: {:.3f}s exceeds threshold: {:.3f}s, Request ID: {}, User: user_{}",
                        (counter%10)*0.1, 0.5, counter*1000 + j, counter%1000);
                    break;
                case 4:
                    log_warn(file_idx, "[SOURCE-2] Resource warning - CPU usage: {:.1f}%, Disk I/O: {:.2f}MB/s, Network: {}Mbps, Temperature: {}C",
                        (counter%100)*0.1, (counter%50)*0.1, (counter%1000)*0.001, 30 + (counter%20));
                    break;
                case 5:
                    log_warn(file_idx, "[SOURCE-2] Database warning - Query execution time: {:.3f}s, Rows affected: {}, Connection pool: {}/{}",
                        (counter%20)*0.05, counter*10, counter%50, 100);
                    break;
                case 6:
                    log_warn(file_idx, "[SOURCE-2] Security warning - Failed login attempts: {}, IP: 192.168.1.{}, User agent: Mozilla/5.0, Timestamp: {}",
                        counter%10, counter%255, log->format_utc_time(log->get_utc_time(), true));
                    break;
                case 7:
                    log_warn(file_idx, "[SOURCE-2] Configuration warning - Invalid config value: '{}' for parameter '{}', using default: '{}'",
                        "invalid_value_" + std::to_string(counter), "param_" + std::to_string(counter%10), "default_value");
                    break;
                case 8:
                    log_warn(file_idx, "[SOURCE-2] File system warning - Disk space: {:.1f}% used ({:.1f}GB/{:.1f}GB), Path: /var/log/app_{}.log",
                        (counter%90)*0.1, (counter%800)*0.1, 100.0, counter%10);
                    break;
                case 9:
                    log_warn(file_idx, "[SOURCE-2] Application warning - Thread pool utilization: {:.1f}%, Active threads: {}/{}, Queue size: {}, Processing time: {:.3f}ms",
                        (counter%100)*0.1, counter%20, 50, counter%1000, (counter%100)*0.1);
                    break;
            }
            counter++;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
    }
}

// 输入源3的线程函数 - 测试csv文件存储
void test_3(zhh_log* log, int file_idx)
{
    static int counter = 0;
    while (g_running.load())
    {
        for(int j = 0; j < 100; j++)
        {
            log_csv(file_idx, "{},{},{},{},{},{},{},{},{},{},{},{}",
                counter, counter+1, counter+2, counter+3, counter+4,
                counter+5, counter+6, counter+7, counter+8, counter+9,
                counter+10, counter+11);
            counter++;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

int main(int argc, char* argv[])
{
    zhh_log* log = zhh_log::instance();
    log->init(4);

    int file_idx = log->open_file("test.log");
    int file_idx2 = log->open_file("test2.log");
    int file_idx3 = log->open_file("test3.csv");

    // 启动输入源1线程
    std::thread thread1(test_1, log, file_idx);

    // 启动输入源2线程
    std::thread thread2(test_2, log, file_idx2);

    // 启动输入源3线程
    std::thread thread3(test_3, log, file_idx3);

    time_t time = std::time(nullptr);

    // 主线程监控缓冲区状态
    while (true)
    {
        time_t current_time = std::time(nullptr);
        if(current_time - time > 1)
        {
            time = current_time;
            printf("remain buffer size: %zu\n", log->get_remain_buffer_size());
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // 清理线程（虽然这里不会执行到，但为了完整性）
    g_running = false;
    thread1.join();
    thread2.join();
    thread3.join();

    return 0;
}
