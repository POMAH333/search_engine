cmake_minimum_required(VERSION 3.15)
project(search_engine VERSION 0.1.2 LANGUAGES C CXX)

set(JSON_BuildTests OFF CACHE INTERNAL "")

add_subdirectory(nlohmann_json)

add_executable(search_engine main.cpp)

target_link_libraries(search_engine PRIVATE nlohmann_json::nlohmann_json)

include(CTest)
enable_testing()

add_compile_definitions(MYPROJECT_VERSION="${PROJECT_VERSION}")

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
