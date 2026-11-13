cmake_minimum_required(VERSION 3.16)
project(skopin LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(skopin
    src/ware.cpp
    src/kiln.cpp
    src/night.cpp
)
target_include_directories(skopin PUBLIC include)
target_compile_options(skopin PRIVATE -Wall -Wextra -Wpedantic)

add_executable(skopin_tests
    tests/main.cpp
    tests/test_ware.cpp
    tests/test_kiln.cpp
    tests/test_night.cpp
)
target_include_directories(skopin_tests PRIVATE tests)
target_link_libraries(skopin_tests PRIVATE skopin)
target_compile_options(skopin_tests PRIVATE -Wall -Wextra -Wpedantic)
