# zhh-log

高性能C++日志库，支持多线程异步写入和循环缓冲区。

## 特性

- **异步写入**: 多线程设计，写入文件使用独立线程
- **循环缓冲区**: 固定大小缓冲区，内存占用可控
- **多文件支持**: 支持同时写入多个日志文件（多线程产生信息，单个独立线程将其写入文件）
- **线程安全**: 使用互斥锁保证线程安全
- **跨平台**: 支持Windows和Linux (测试环境：ubuntu-20.04.6-desktop-amd64)

## 快速开始

### 编译

**Windows:**
```bash
cd bin
.\build.bat
```

**Linux:**
```bash
cd bin
chmod +x build.sh
./build.sh
```

### 基本使用

```cpp
#include "zhh-log.h"

int main() {
    // 初始化日志系统
    zhh_log::instance()->init();

    // 打开日志文件
    int log_id = zhh_log::instance()->open_file("app.log");

    // 写入日志
    log_info(log_id, "应用程序启动");
    log_warn(log_id, "警告信息: {}", "内存使用率较高");
    log_error(log_id, "错误代码: {}", 404);

    // 原始数据写入
    log_csv(log_id, "数据,时间,值\n{}, {}, {}", "A001", "2024-01-01", 100);

    // 销毁日志系统
    zhh_log::instance()->destroy();
    return 0;
}
```

### 完整测试

详细的完整测试请参考：[main.cpp](src/main.cpp)

### 多线程流程图

系统架构和线程交互流程请参考：[FLOWCHART.md](doc/FLOWCHART.md)

## API 参考

详细的API文档请参考：[API.md](doc/API.md)

## 注意事项

1. 默认存储至data文件夹（没有需自行创建）
2. 编译时需添加fmt选项，实际fmt源代码
3. 根据缓冲区大小实时用量（get_remain_buffer_size()）调整预设值

## 依赖

- C++17 或更高版本
- fmt库（头文件模式）
- pthread（Linux）
