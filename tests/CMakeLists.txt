cmake_minimum_required(VERSION 3.15)

project(server_test)

# get GTest from github
include(FetchContent)
FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
        DOWNLOAD_EXTRACT_TIMESTAMP TRUE
        GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(server_test index_tests.cpp server_tests.cpp
        ${CMAKE_SOURCE_DIR}/src/inverted_index.cpp
        ${CMAKE_SOURCE_DIR}/src/server.cpp)

target_include_directories(server_test PUBLIC ${gtest_SOURCE_DIR}/include ${INCLUDE_DIR})

target_link_libraries(server_test PRIVATE gtest_main)
