# zhh-log API 参考

## 类定义

### zhh_log

单例模式的日志类，提供异步日志写入功能。

## 公共方法

### 初始化和销毁

#### `static zhh_log* instance()`
获取单例实例。

**返回值:**
- `zhh_log*`: 日志实例指针

#### `bool init(int cpu_core = -1, std::string path = "../data/")`
初始化日志系统。

**参数:**
- `cpu_core`: CPU核心绑定（-1为不绑定）
- `path`: 日志文件存储路径

**返回值:**
- `bool`: 初始化是否成功

#### `void destroy()`
销毁日志系统，停止后台线程。

### 文件操作

#### `int open_file(std::string file_name)`
打开日志文件。

**参数:**
- `file_name`: 文件名

**返回值:**
- `int`: 文件ID（0表示失败）

### 日志写入

#### `template<typename... Args> void log_message(int cur_idx, const std::string& level, const std::string& message, Args&&... args)`
写入格式化日志消息。

**参数:**
- `cur_idx`: 文件ID
- `level`: 日志级别
- `message`: 消息模板
- `args...`: 格式化参数

#### `template<typename... Args> void log_original(int cur_idx, const std::string& message, Args&&... args)`
写入原始数据（不添加时间戳和级别）。

### 日志宏

#### `log_info(cur_idx, message, ...)`
写入信息级别日志。

#### `log_warn(cur_idx, message, ...)`
写入警告级别日志。

#### `log_error(cur_idx, message, ...)`
写入错误级别日志。

#### `log_csv(cur_idx, message, ...)`
写入原始CSV数据。

### 缓冲区管理

#### `bool write_to_buffer(const LogItem& log)`
写入日志到缓冲区。

**参数:**
- `log`: 日志项

**返回值:**
- `bool`: 写入是否成功

#### `bool read_from_buffer(LogItem& log_read, std::chrono::milliseconds timeout = std::chrono::milliseconds(100))`
从缓冲区读取日志。

**参数:**
- `log_read`: 输出日志项
- `timeout`: 超时时间

**返回值:**
- `bool`: 读取是否成功

#### `size_t get_valid_log_num() const`
获取有效日志数量。

**返回值:**
- `size_t`: 待处理日志数量

#### `size_t get_remain_buffer_size() const`
获取剩余缓冲区大小。

**返回值:**
- `size_t`: 剩余缓冲区大小

#### `void clear_buffer()`
清空缓冲区。

### 时间工具

#### `double get_utc_time()`
获取UTC时间。

**返回值:**
- `double`: UTC时间戳

#### `std::string format_utc_time(double t, bool with_millis = false)`
格式化UTC时间。

**参数:**
- `t`: 时间戳
- `with_millis`: 是否包含毫秒

**返回值:**
- `std::string`: 格式化时间字符串

## 数据结构

### LogItem

```cpp
struct LogItem {
    int file_idx;           // 文件ID
    std::string message;    // 日志消息

    LogItem();              // 默认构造函数
    LogItem(int idx, std::string msg);  // 带参数构造函数
};
```

## 配置参数

### 私有成员

- `size_t m_buffer_size{10000}`: 缓冲区大小
- `std::string m_file_path`: 文件路径
- `int m_cpu_core`: CPU核心绑定

## 错误处理

- 文件打开失败返回0
- 缓冲区满时丢弃新日志
- 线程绑定失败会输出错误信息

## 线程安全

所有公共方法都是线程安全的，内部使用互斥锁保护。
