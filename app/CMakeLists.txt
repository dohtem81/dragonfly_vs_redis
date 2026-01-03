cmake_minimum_required(VERSION 3.10)
project(RedisPubSub)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BINARY_DIR /var/build)
set(CMAKE_CURRENT_SOURCE_DIR /usr/src/app/src)

# Find hiredis library
find_library(HIREDIS_LIB hiredis)
if(NOT HIREDIS_LIB)
    message(FATAL_ERROR "hiredis library not found")
endif()

# OpenMP package
find_package(OpenMP REQUIRED)
if(NOT HIREDIS_LIB)
    message(FATAL_ERROR "openmp package not found")
endif()

# Threads package
find_package(Threads REQUIRED)

# Combined publisher (replaces pub_dragonfly and pub_redis)
add_executable(pub_combined ${CMAKE_CURRENT_SOURCE_DIR}/pub_combined.cpp)
target_link_libraries(pub_combined ${HIREDIS_LIB} OpenMP::OpenMP_CXX)

add_executable(sub_both ${CMAKE_CURRENT_SOURCE_DIR}/sub_both.cpp)
target_link_libraries(sub_both ${HIREDIS_LIB} Threads::Threads)
