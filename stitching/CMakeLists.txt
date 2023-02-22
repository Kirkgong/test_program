cmake_minimum_required(VERSION 3.10)
project(stitch)

set(APP_SRC main.cpp)
add_executable(${PROJECT_NAME} ${APP_SRC})

add_definitions(-DDEBUG=1)
#print message
# message(${PROJECT_SOURCE_DIR})