#!/bin/bash

# 检查是否安装了 clang-format
if ! command -v clang-format &> /dev/null; then
    echo "错误: 未找到 clang-format，请先安装"
    echo "在 Ubuntu 上可以使用: sudo apt-get install clang-format"
    exit 1
fi

# 查找所有 C++ 源文件
find . -type f \( -name "*.cpp" -o -name "*.h" -o -name "*.hpp" \) \
    -not -path "./build/*" \
    -not -path "./.git/*" \
    -exec clang-format -i {} \;

echo "代码格式化完成！" 