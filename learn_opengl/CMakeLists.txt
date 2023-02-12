cmake_minimum_required(VERSION 3.0...3.20 FATAL_ERROR)
project(learn_opengl)

include_directories(./glad/include)
include_directories(./glfw-3.3.8/include)
include_directories(./stb-image)

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD 11)

set(APP_SRC main.cpp ./glad/src/glad.c ./stb-image/stb_image.cpp)
#set(APP_SRC main.cpp ./glad/src/glad.c )

link_directories(./glfw-3.3.8/build/src) 

add_executable(${PROJECT_NAME} ${APP_SRC})

target_link_libraries(${PROJECT_NAME} -lglfw3) 
target_link_libraries(${PROJECT_NAME} -ldl) 
target_link_libraries(${PROJECT_NAME} -lX11) 
target_link_libraries(${PROJECT_NAME} -lpthread) 

add_definitions(-DDEBUG=1)
#print message
message(${PROJECT_SOURCE_DIR})
