project(mp4_repair)
set(APP_SRC main.cpp Mp4Repair.cpp)
add_executable(${PROJECT_NAME} main.cpp  Mp4Repair.cpp)
# include_directories(/home/nio/project/luke-dvr/src/luke-dvr/src/IncidentVideo)
add_definitions(-DDEBUG=1)
#print message
message(${PROJECT_SOURCE_DIR})