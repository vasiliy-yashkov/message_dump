cmake_minimum_required(VERSION 3.0.0)
project(message_dump VERSION 0.1.0)

if(NOT DEFINED ENV{FB_HOME})
    message(FATAL_ERROR "Variable FB_HOME must be set")
endif()

set(FB_HOME $ENV{FB_HOME})

include_directories(${FB_HOME}/include)

include(CTest)
enable_testing()

add_executable(message_dump main.cpp)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
