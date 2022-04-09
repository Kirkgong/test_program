project(mem_test)
set(APP_SRC main.cpp /home/nio/project/luke-dvr/src/luke-dvr/src/IncidentVideo/AudioData.cpp)
add_executable(${PROJECT_NAME} main.cpp  /home/nio/project/luke-dvr/src/luke-dvr/src/IncidentVideo/AudioData.cpp)
include_directories(/home/nio/project/luke-dvr/src/luke-dvr/src/IncidentVideo)
add_definitions(-DDEBUG=1)
#print message
message(${PROJECT_SOURCE_DIR})