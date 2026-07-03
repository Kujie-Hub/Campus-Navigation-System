cmake_minimum_required(VERSION 3.10)
project(CampusNavigation)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR})

include_directories(include)

add_executable(campus_server 
    src/server_main.cpp 
    src/http_server.cpp
    src/path_planner.cpp
)

target_compile_features(campus_server PRIVATE cxx_std_11)

if(WIN32)
    target_link_libraries(campus_server ws2_32)
endif()
