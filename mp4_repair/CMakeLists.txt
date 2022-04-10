cmake_minimum_required(VERSION 3.10)
project(mp4_repair)

include_directories(/home/nio/project/ffmpeg)
set(APP_SRC main.cpp Mp4Repair.cpp FileMux.cpp)
link_directories(/home/nio/project/ffmpeg/libavformat) 
link_directories(/home/nio/project/ffmpeg/libavcodec) 
add_executable(${PROJECT_NAME} main.cpp  Mp4Repair.cpp FileMux.cpp)
target_link_libraries(${PROJECT_NAME} -lavformat ) 
target_link_libraries(${PROJECT_NAME} -lavcodec ) 

add_definitions(-DDEBUG=1)
#print message
message(${PROJECT_SOURCE_DIR})