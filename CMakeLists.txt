# Setez versiunea minima pt cmake
cmake_minimum_required(VERSION 3.10)

# Numele proiectului
project(TEMA5)

# Setare versiune C
set(CMAKE_C_STANDARD 17)

# Creare executabile
add_executable(server src/server.c)
add_executable(client src/client.c)