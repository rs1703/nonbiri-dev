cmake_minimum_required(VERSION 3.15)

include("cmake/HunterGate.cmake")
HunterGate(
  URL "https://github.com/cpp-pm/hunter/archive/v0.24.0.tar.gz"
  SHA1 "a3d7f4372b1dcd52faa6ff4a3bd5358e1d0e5efd"
  LOCAL)

project(nonbiri-dev LANGUAGES C CXX)
include(FindPkgConfig)

# Deps =======================================

file(GLOB GUMBO_QUERY_SOURCES libs/nonbiri-core-dev/libs/gumbo-query/src/*.cpp)
set(DEPS
  ${GUMBO_QUERY_SOURCES}
  libs/nonbiri-core-dev/core/extension/extension.cpp
  libs/nonbiri-core-dev/core/parser/parser.cpp
  libs/nonbiri-core-dev/core/utils/utils.cpp)

include_directories(
  ${CMAKE_CURRENT_LIST_DIR}
  libs/nonbiri-core-dev/libs
  libs/nonbiri-core-dev)

# Main =======================================

file(GLOB MAIN_SOURCES nonbiri/*.cpp nonbiri/*/*.cpp libs/cpp-httplib/httplib.h)
add_executable(${PROJECT_NAME} ${DEPS} ${MAIN_SOURCES})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
target_include_directories(${PROJECT_NAME} PRIVATE libs/cpp-httplib)

# Indexer ====================================

file(GLOB INDEX_SOURCES
  nonbiri/models/*.cpp
  nonbiri/manager.cpp
  nonbiri/atomic_map.cpp
  nonbiri/utils.cpp
  nonbiri/lru.cpp
  indexer/*.cpp)
add_executable(index ${DEPS} ${INDEX_SOURCES})
target_compile_features(index PRIVATE cxx_std_17)

# Packages ===================================

add_definitions(-DCURL_STATICLIB)
pkg_check_modules(CURL libcurl)
if(CURL_FOUND)
  include_directories(${CURL_INCLUDE_DIR})

  target_link_libraries(${PROJECT_NAME} PRIVATE ${CURL_LIBRARIES})
  target_link_libraries(index PRIVATE ${CURL_LIBRARIES})
else()
  unset(CURL_FOUND CACHE)
  hunter_add_package(CURL)
  find_package(CURL CONFIG REQUIRED)

  target_link_libraries(${PROJECT_NAME} PRIVATE CURL::libcurl)
  target_link_libraries(index PRIVATE CURL::libcurl)
endif()

pkg_check_modules(SQLITE3 sqlite3)
if(SQLITE3_FOUND)
  include_directories(${SQLITE3_INCLUDE_DIR})

  target_link_libraries(${PROJECT_NAME} PRIVATE ${SQLITE3_LIBRARIES})
else()
  unset(SQLITE3_FOUND CACHE)
  hunter_add_package(sqlite3)
  find_package(sqlite3 CONFIG REQUIRED)

  target_link_libraries(${PROJECT_NAME} PRIVATE sqlite3::sqlite3)
endif()

pkg_check_modules(JSONCPP jsoncpp)
if(JSONCPP_FOUND)
  include_directories(${JSONCPP_INCLUDE_DIR})

  target_link_libraries(${PROJECT_NAME} PRIVATE ${JSONCPP_LIBRARIES})
  target_link_libraries(index PRIVATE ${JSONCPP_LIBRARIES})
else()
  unset(JSONCPP_FOUND CACHE)
  hunter_add_package(jsoncpp)
  find_package(jsoncpp CONFIG REQUIRED)

  target_link_libraries(${PROJECT_NAME} PRIVATE jsoncpp_lib_static)
  target_link_libraries(index PRIVATE jsoncpp_lib_static)
endif()

pkg_check_modules(GUMBO gumbo)
if(GUMBO_FOUND)
  include_directories(${GUMBO_INCLUDE_DIR})

  target_link_libraries(${PROJECT_NAME} PRIVATE ${GUMBO_LIBRARIES})
  target_link_libraries(index PRIVATE ${GUMBO_LIBRARIES})
else()
  unset(GUMBO_FOUND CACHE)
  hunter_add_package(gumbo)
  find_package(gumbo CONFIG REQUIRED)

  target_link_libraries(${PROJECT_NAME} PRIVATE gumbo::gumbo)
  target_link_libraries(index PRIVATE gumbo::gumbo)
endif()