cmake_minimum_required(VERSION 3.15)

project(search_engine VERSION 1.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

set(INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
set(RESOURCES ${CMAKE_SOURCE_DIR}/resources)

add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(nlohmann_json)

target_compile_definitions(search_engine PRIVATE 
    MYPROJECT_VERSION="${PROJECT_VERSION}"
)