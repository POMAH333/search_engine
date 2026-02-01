cmake_minimum_required(VERSION 3.15)

set(JSON_BuildTests OFF CACHE INTERNAL "")

configure_file(${CMAKE_SOURCE_DIR}/config/config.json ${CMAKE_CURRENT_BINARY_DIR})
configure_file(${CMAKE_SOURCE_DIR}/config/requests.json ${CMAKE_CURRENT_BINARY_DIR})
configure_file(${CMAKE_SOURCE_DIR}/config/answer.json ${CMAKE_CURRENT_BINARY_DIR})

add_executable(search_engine main.cpp converter.cpp inverted_index.cpp server.cpp)

target_include_directories(search_engine PUBLIC  ${INCLUDE_DIR})

target_link_libraries(search_engine PRIVATE nlohmann_json::nlohmann_json)
