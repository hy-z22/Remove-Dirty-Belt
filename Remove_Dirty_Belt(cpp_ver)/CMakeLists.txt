cmake_minimum_required(VERSION 3.10)
project(Remove_Dirty_Belt LANGUAGES CXX)

# 开启生成 compile_commands.json（用于 VSCode IntelliSense）
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 设置默认构建类型为 Debug（除非用户指定）
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build." FORCE)
endif()

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加可执行文件
add_executable(${PROJECT_NAME}
    src/main.cpp
    src/remove_belt.cpp
    src/utils.cpp
)

# 包含头文件路径
target_include_directories(${PROJECT_NAME} PUBLIC
    ${CMAKE_SOURCE_DIR}/include
)

# 编译选项
target_compile_options(${PROJECT_NAME} PRIVATE
    -Wall -Wextra
)