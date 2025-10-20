#include "zhh-log.h"
#include <mutex>

#ifdef _WIN32
#include <windows.h>
#else
#include <pthread.h>
#include <sched.h>
#endif

#if defined _WIN32 |  defined _WIN64
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

#include <sys/timeb.h>
#include <ctime>

zhh_log* zhh_log::s_instance = nullptr;

zhh_log::zhh_log()
{

}

zhh_log::~zhh_log()
{
    destroy();
}

zhh_log* zhh_log::instance()
{
    if(s_instance == nullptr)
    {
        s_instance = new zhh_log();
    }
    return s_instance;
}

bool zhh_log::init(int cpu_core, std::string path)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    if (m_running.load())
    {
        return false;
    }

    m_buffer.resize(m_buffer_size);

    m_file_path = path;
    m_file_path.append("data-");
    m_file_path.append(format_utc_time(get_utc_time()));
    #if defined _WIN32 |  defined _WIN64
    _mkdir(m_file_path.c_str());
    #else
    mkdir(m_file_path.c_str(), 0755);
    #endif

    m_cpu_core = cpu_core;
    m_running = true;
    m_thread = std::thread(&zhh_log::loop, this);

    return true;
}

int zhh_log::open_file(std::string file_name)
{
    std::lock_guard<std::mutex> lock(m_mutex);

    std::string file_path = m_file_path;
    file_path.append("/");
    file_path.append(file_name);

    FILE* file = nullptr;
    #if defined _WIN32 |  defined _WIN64
    fopen_s(&file, file_path.c_str(), "wt");
    #else
    file = fopen(file_path.c_str(), "wt");
    #endif
    if (file)
    {
        m_idx_file_map[++m_file_idx] = file;
        return m_file_idx;
    }

    return 0;
}

void zhh_log::destroy()
{
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        if (!m_running.load())
        {
            return;
        }

        m_running = false;
    }

    m_cv.notify_all();

    if (m_thread.joinable())
    {
        m_thread.join();
    }

    printf("[%s] The simulation zhh_log has been stopped.\n", __FUNCTION__);
}


void zhh_log::loop()
{
    // 在线程内部绑核 - bind core in the thread
    if (m_cpu_core >= 0)
    {
        bindCurrentThreadToCore(m_cpu_core);
    }

    while (m_running.load())
    {

        LogItem log = {0, ""};
        if (read_from_buffer(log))
        {
            auto iter = m_idx_file_map.find(log.file_idx);
            if (iter != m_idx_file_map.end())
            {
                fputs(log.message.c_str(), iter->second);
                fflush(iter->second);
            }
        }

        if (!m_running.load())
        {
            break;
        }
    }
}

bool zhh_log::bindCurrentThreadToCore(int cpu_core)
{
#ifdef _WIN32
    DWORD_PTR mask = 1ULL << cpu_core;
    DWORD_PTR result = SetThreadAffinityMask(GetCurrentThread(), mask);

    if (result == 0) {
        DWORD error = GetLastError();
        printf("[%s] SetThreadAffinityMask failed, error: %lu\n", __FUNCTION__, error);
        return false;
    }
    return true;
#else
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(cpu_core, &cpuset);

    int result = pthread_setaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);

    if (result != 0) {
        printf("[%s] pthread_setaffinity_np failed, error: %d\n", __FUNCTION__, result);
        return false;
    }
    return true;
#endif
}

double zhh_log::get_utc_time()
{
    double ret;
    struct timeb SysTime;
    ftime(&SysTime);
    ret = SysTime.time + SysTime.millitm / 1000.0;
    return ret;
}

std::string zhh_log::format_utc_time(double t, bool with_millis)
{
    time_t SecCnt = (time_t)t;
    struct tm unitime;
    #if defined _WIN32 |  defined _WIN64
    localtime_s(&unitime, &SecCnt);
    #else
    localtime_r(&SecCnt, &unitime);
    #endif
    unsigned short year      = unitime.tm_year + 1900;
    unsigned char  month     = unitime.tm_mon + 1;
    unsigned char  day       = unitime.tm_mday;
    unsigned char  hour      = unitime.tm_hour;
    unsigned char  minute    = unitime.tm_min;
    unsigned char  secs      = unitime.tm_sec;
    unsigned char  millis    = (t - SecCnt) * 1000;
    char buf[64];
    if (with_millis)
    {
        sprintf(buf, "%04d/%02d/%02d-%02d:%02d:%02d.%03d", year, month, day, hour, minute, secs, millis);
    }
    else
    {
        sprintf(buf, "%04d%02d%02d-%02d%02d%02d", year, month, day, hour, minute, secs);
    }
    return std::string(buf);
}


size_t zhh_log::get_valid_log_num() const
{
    size_t write_idx = m_write_index.load();
    size_t read_idx = m_read_index.load();

    return (write_idx - read_idx + m_buffer_size) % m_buffer_size;
}

size_t zhh_log::get_remain_buffer_size() const
{
    return m_buffer_size - get_valid_log_num();
}

void zhh_log::clear_buffer()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_write_index.store(0);
    m_read_index.store(0);
}

bool zhh_log::write_to_buffer(const LogItem& log)
{
    // std::unique_lock<std::mutex> lock(m_mutex);
    std::lock_guard<std::mutex> lock(m_mutex);

    size_t write_idx = m_write_index.load();
    size_t next_write_idx = (write_idx + 1) % m_buffer_size;

    // 检查缓冲区是否已满 - check if the buffer is full
    if (next_write_idx == m_read_index.load())
        return false;

    m_buffer[write_idx] = log;

    m_write_index.store(next_write_idx);

    m_cv.notify_one();

    return true;
}

bool zhh_log::read_from_buffer(LogItem& log_read, std::chrono::milliseconds timeout)
{
    std::unique_lock<std::mutex> lock(m_mutex);

    if (m_cv.wait_for(lock, timeout, [this] { return get_valid_log_num() > 0; }))
    {
        size_t read_idx = m_read_index.load();

        // 检查是否有数据可读 - check if there is data to read
        if (read_idx == m_write_index.load())
            return false;

        log_read = m_buffer[read_idx];

        m_read_index.store((read_idx + 1) % m_buffer_size);

        return true;
    }

    return false;
}
