cmake_minimum_required(VERSION 3.5)

find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK ccache)
endif(CCACHE_FOUND)

include("cmake/HunterGate.cmake")
HunterGate(
  URL "https://github.com/cpp-pm/hunter/archive/v0.23.297.tar.gz"
  SHA1 "3319fe6a3b08090df7df98dee75134d68e2ef5a3"
)

project(nonbiri-dev)
enable_language(C CXX)

hunter_add_package(CURL)
hunter_add_package(gumbo)
hunter_add_package(jsoncpp)
hunter_add_package(sqlite3)

find_package(CURL CONFIG REQUIRED)
find_package(gumbo CONFIG REQUIRED)
find_package(jsoncpp CONFIG REQUIRED)
find_package(sqlite3 CONFIG REQUIRED)

include_directories(
  ${CMAKE_CURRENT_LIST_DIR}/libs/nonbiri-core-dev/libs
  ${CMAKE_CURRENT_LIST_DIR}/libs/nonbiri-core-dev
  ${CMAKE_CURRENT_LIST_DIR}
  ${CURL_ROOT}/include
  ${GUMBO_ROOT}/include
)

set(deps
  ${CMAKE_CURRENT_LIST_DIR}/libs/nonbiri-core-dev/libs/gumbo-query/src/Document.cpp
  ${CMAKE_CURRENT_LIST_DIR}/libs/nonbiri-core-dev/libs/gumbo-query/src/Node.cpp
  ${CMAKE_CURRENT_LIST_DIR}/libs/nonbiri-core-dev/libs/gumbo-query/src/Object.cpp
  ${CMAKE_CURRENT_LIST_DIR}/libs/nonbiri-core-dev/libs/gumbo-query/src/Parser.cpp
  ${CMAKE_CURRENT_LIST_DIR}/libs/nonbiri-core-dev/libs/gumbo-query/src/QueryUtil.cpp
  ${CMAKE_CURRENT_LIST_DIR}/libs/nonbiri-core-dev/libs/gumbo-query/src/Selection.cpp
  ${CMAKE_CURRENT_LIST_DIR}/libs/nonbiri-core-dev/libs/gumbo-query/src/Selector.cpp
  ${CMAKE_CURRENT_LIST_DIR}/libs/nonbiri-core-dev/core/extension/extension.cpp
  ${CMAKE_CURRENT_LIST_DIR}/libs/nonbiri-core-dev/core/parser/parser.cpp
  ${CMAKE_CURRENT_LIST_DIR}/libs/nonbiri-core-dev/core/utils/utils.cpp
)

file (GLOB sources nonbiri/*.cpp nonbiri/*/*.cpp)
add_executable(${PROJECT_NAME} ${deps} ${sources})

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_link_libraries(${PROJECT_NAME} PUBLIC CURL::libcurl gumbo::gumbo sqlite3::sqlite3 jsoncpp_lib_static)

# indexer

file (GLOB index_sources indexer/*.cpp nonbiri/cache/*.cpp nonbiri/manager/*.cpp nonbiri/models/*.cpp nonbiri/utils/*.cpp)
add_executable(index ${deps} ${index_sources})

target_compile_features(index PRIVATE cxx_std_17)
target_link_libraries(index PUBLIC CURL::libcurl gumbo::gumbo jsoncpp_lib_static)