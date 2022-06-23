cmake_minimum_required(VERSION 3.15)

include("cmake/HunterGate.cmake")
HunterGate(
  URL "https://github.com/cpp-pm/hunter/archive/v0.24.0.tar.gz"
  SHA1 "a3d7f4372b1dcd52faa6ff4a3bd5358e1d0e5efd"
  LOCAL)

project(nonbiri LANGUAGES C CXX)
include(FindPkgConfig)

add_definitions(-DCURL_STATICLIB)
pkg_check_modules(CURL libcurl)
if(CURL_FOUND)
  include_directories(${CURL_INCLUDE_DIR})

  set(LIBRARIES ${LIBRARIES} ${CURL_LIBRARIES})
else()
  unset(CURL_FOUND CACHE)
  hunter_add_package(CURL)
  find_package(CURL CONFIG REQUIRED)

  set(LIBRARIES ${LIBRARIES} CURL::libcurl)
endif()

pkg_check_modules(SQLITE3 sqlite3)
if(SQLITE3_FOUND)
  include_directories(${SQLITE3_INCLUDE_DIR})

  set(LIBRARIES ${LIBRARIES} ${SQLITE3_LIBRARIES})
else()
  unset(SQLITE3_FOUND CACHE)
  hunter_add_package(sqlite3)
  find_package(sqlite3 CONFIG REQUIRED)

  set(LIBRARIES ${LIBRARIES} sqlite3::sqlite3)
endif()

pkg_check_modules(JSONCPP jsoncpp)
if(JSONCPP_FOUND)
  include_directories(${JSONCPP_INCLUDE_DIR})

  set(LIBRARIES ${LIBRARIES} ${JSONCPP_LIBRARIES})
else()
  unset(JSONCPP_FOUND CACHE)
  hunter_add_package(jsoncpp)
  find_package(jsoncpp CONFIG REQUIRED)

  set(LIBRARIES ${LIBRARIES} jsoncpp_lib_static)
endif()

pkg_check_modules(GUMBO gumbo)
if(GUMBO_FOUND)
  include_directories(${GUMBO_INCLUDE_DIR})

  set(LIBRARIES ${LIBRARIES} ${GUMBO_LIBRARIES})
else()
  unset(GUMBO_FOUND CACHE)
  hunter_add_package(gumbo)
  find_package(gumbo CONFIG REQUIRED)

  set(LIBRARIES ${LIBRARIES} gumbo::gumbo)
endif()

file(GLOB CORE_SOURCES
  ${CMAKE_CURRENT_LIST_DIR}/libs/nonbiri-core-dev/core/*.cpp
  ${CMAKE_CURRENT_LIST_DIR}/libs/nonbiri-core-dev/core/*/*.cpp
  ${CMAKE_CURRENT_LIST_DIR}/libs/nonbiri-core-dev/libs/gumbo-query/src/*.cpp)
set(DEPS ${CORE_SOURCES})

include_directories(
  ${CMAKE_CURRENT_LIST_DIR}
  ${CMAKE_CURRENT_LIST_DIR}/libs/nonbiri-core-dev/libs
  ${CMAKE_CURRENT_LIST_DIR}/libs/nonbiri-core-dev)

file(GLOB MAIN_SOURCES 
  ${CMAKE_CURRENT_LIST_DIR}/${PROJECT_NAME}/*.cpp
  ${CMAKE_CURRENT_LIST_DIR}/${PROJECT_NAME}/*/*.cpp)

add_executable(${PROJECT_NAME} ${DEPS} ${MAIN_SOURCES})
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)
target_include_directories(${PROJECT_NAME} PRIVATE libs/cpp-httplib)

if(WIN32)
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBRARIES})
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_CURRENT_LIST_DIR}/${PROJECT_NAME}/schema.sql"
    "${CMAKE_CURRENT_BINARY_DIR}/Release/${PROJECT_NAME}.sql")
else()
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBRARIES} -ldl)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
    "${CMAKE_CURRENT_LIST_DIR}/${PROJECT_NAME}/schema.sql"
    "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.sql")
endif()
