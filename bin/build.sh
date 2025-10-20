#!/bin/bash
# 简化版Linux编译脚本

# 切换到项目根目录
cd "$(dirname "$0")/.."

# 创建输出目录
mkdir -p bin

# 编译
echo "编译 zhh_log..."
g++ -std=c++17 -Wall -O2 -DFMT_HEADER_ONLY -I./src src/*.cpp -lpthread -o bin/zhh_log

# 检查结果
if [ $? -eq 0 ]; then
    echo "编译成功: bin/zhh_log"
    chmod +x bin/zhh_log
else
    echo "编译失败!"
    exit 1
fi
