cmake_minimum_required(VERSION 3.15)
project(search_engine VERSION 0.1.2 LANGUAGES C CXX)

include(CTest)
enable_testing()

if(POLICY CMP0135)
    cmake_policy(SET CMP0135 NEW)
endif()

set(JSON_BuildTests OFF CACHE INTERNAL "")

add_subdirectory(nlohmann_json)          

include(FetchContent)
# FetchContent_Declare(
#     googletest
#     URL https://github.com/google/googletest/archive/609281088cfefc76f9d0ce82e1ff6c30cc3591e5.zip
#     DOWNLOAD_EXTRACT_TIMESTAMP TRUE
#     GIT_SHALLOW TRUE
# )

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest
    GIT_TAG release-1.12.1
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    GIT_SHALLOW TRUE
)

if(WIN32 OR MINGW)
    set(gtest_disable_pthreads ON CACHE BOOL "" FORCE)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
endif()

FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(search_engine main.cpp)

target_link_libraries(search_engine PRIVATE nlohmann_json::nlohmann_json)

target_compile_definitions(search_engine PRIVATE 
    MYPROJECT_VERSION="${PROJECT_VERSION}"
)

include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

target_link_libraries(search_engine PRIVATE gtest_main)

include(GoogleTest)
gtest_discover_tests(search_engine)

add_test(NAME all_tests COMMAND search_engine --gtest)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
