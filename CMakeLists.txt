cmake_minimum_required(VERSION 3.10)

project(Proiect)

set(CMAKE_CXX_STANDARD 20)

# Find OpenCV package
find_package(OpenCV REQUIRED)

# Include directories for OpenCV
include_directories(${OpenCV_INCLUDE_DIRS})

# Create executables
add_executable(server src/server.cpp)
add_executable(client1 src/client1.cpp)
add_executable(clienta src/clienta.cpp)
add_executable(contur src/contur.cpp)
add_executable(canny src/canny.cpp)
add_executable(rotate src/rotate.cpp)

# Link OpenCV libraries
target_link_libraries(contur ${OpenCV_LIBS})
target_link_libraries(canny ${OpenCV_LIBS})
target_link_libraries(rotate ${OpenCV_LIBS})
