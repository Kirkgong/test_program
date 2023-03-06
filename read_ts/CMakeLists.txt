cmake_minimum_required(VERSION 3.10)
project(read_ts)

set(APP_SRC main.cpp PraseTsFile.cpp TSFormat.cpp)
add_executable(${PROJECT_NAME} ${APP_SRC})

add_definitions(-DDEBUG=1)
#print message
message(${PROJECT_SOURCE_DIR})