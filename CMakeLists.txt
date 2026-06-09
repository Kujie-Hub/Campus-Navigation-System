cmake_minimum_required(VERSION 3.10)
project(CampusNavigation)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

include_directories(include)

add_executable(campus_navigation 
    src/main.cpp 
    src/path_planner.cpp
)

target_compile_features(campus_navigation PRIVATE cxx_std_11)