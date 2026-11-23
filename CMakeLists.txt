cmake_minimum_required(VERSION 3.21)
project(tp_tcp_simple LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Network)
qt_standard_project_setup()

qt_add_executable(serveur serveur/main.cpp)
target_link_libraries(serveur PRIVATE Qt6::Core Qt6::Network)

qt_add_executable(client client/main.cpp)
target_link_libraries(client PRIVATE Qt6::Core Qt6::Network)
