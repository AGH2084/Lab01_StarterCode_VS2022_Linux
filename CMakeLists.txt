cmake_minimum_required(VERSION 3.21)
project(PP2026_Lab01_Assignment01 LANGUAGES CXX)

# One source code base for Visual Studio 2022 (MSVC) and Linux (GCC/Clang).
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(OpenMP REQUIRED COMPONENTS CXX)
find_package(Threads REQUIRED)

add_executable(omp_team_explorer omp_team_explorer.cpp)
target_link_libraries(omp_team_explorer PRIVATE OpenMP::OpenMP_CXX)

add_executable(work_models work_models.cpp)
target_link_libraries(work_models PRIVATE OpenMP::OpenMP_CXX Threads::Threads)

foreach(target IN ITEMS omp_team_explorer work_models)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endforeach()
