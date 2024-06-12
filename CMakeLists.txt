# Setez versiunea minima pt cmake
cmake_minimum_required(VERSION 3.10)

# Numele proiectului
project(Proiect)

# Setare versiune C
set(CMAKE_C_STANDARD 17)

# Creare executabile
add_executable(server src/server.cpp)
add_executable(client1 src/client1.cpp)
add_executable(clienta src/clienta.cpp)
add_executable(contur src/contur.cpp)
add_executable(procesareImagine src/procesareImagine.cpp)
add_executable(rotate src/rotate.cpp)
