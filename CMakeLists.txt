cmake_minimum_required(VERSION 3.16)
project(TravelAgencyConsole LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(TravelAgency
    src/main.cpp
    src/Booking.cpp
    src/TravelAgency.cpp
)

target_include_directories(TravelAgency PRIVATE include third_party)
