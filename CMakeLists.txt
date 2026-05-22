cmake_minimum_required(VERSION 3.20)
project(Grundschule LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(PkgConfig REQUIRED)
pkg_check_modules(SFML REQUIRED IMPORTED_TARGET
    sfml-graphics sfml-window sfml-system)

# Shared common library
add_library(common STATIC
    src/common/common.cpp
)
target_include_directories(common PUBLIC src/common)
target_link_libraries(common PUBLIC PkgConfig::SFML)

# Helper function to add an application
function(add_app name)
    file(GLOB APP_SOURCES src/${name}/*.cpp)
    add_executable(${name} ${APP_SOURCES})
    target_link_libraries(${name} PRIVATE common)
endfunction()

add_app(rechnen)
add_app(schreiben)
add_app(schiebepuzzle)
add_app(geburtstagsrechner)
add_app(kalender)
add_app(waehrung)
add_app(frisches_wasser)
add_app(lesen)