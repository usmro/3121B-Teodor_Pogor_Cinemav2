cmake_minimum_required(VERSION 3.16)
project(cinema)

set(CMAKE_CXX_STANDARD 17)

find_package(SFML 3 COMPONENTS Graphics Window System REQUIRED)

add_executable(cinema
    src/main.cpp
    src/Film.cpp
    src/Sala.cpp
    src/Rezervare.cpp
    src/Cinematograf.cpp
)

target_link_libraries(cinema SFML::Graphics SFML::Window SFML::System)