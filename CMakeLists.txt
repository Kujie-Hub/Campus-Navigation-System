cmake_minimum_required(VERSION 3.10)
project(CampusNavigation)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(include)

# 原有导航程序
add_executable(campus_navigation 
    src/main.cpp 
    src/path_planner.cpp
)

# HTTP服务器（前后端集成）
add_executable(campus_server 
    src/server_main.cpp 
    src/http_server.cpp
)

# 单元测试程序
add_executable(test_path_planner
    tests/test_path_planner.cpp
    src/path_planner.cpp
)

target_compile_features(campus_navigation PRIVATE cxx_std_11)
target_compile_features(campus_server PRIVATE cxx_std_11)
target_compile_features(test_path_planner PRIVATE cxx_std_11)
