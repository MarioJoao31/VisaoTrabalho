cmake_minimum_required(VERSION 3.5)
project(my_project)

# Find OpenCV package
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

# Set C++ standard
set(CMAKE_CXX_STANDARD 11)

# Add executable
add_executable(run main.cpp vc.c )
target_link_libraries(run ${OpenCV_LIBS})