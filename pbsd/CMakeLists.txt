cmake_minimum_required(VERSION 3.28)
project(PBSD LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Kernel code: no exceptions, no RTTI
set(KERNEL_CXX_FLAGS "-fno-exceptions -fno-rtti -ffreestanding -fno-stack-protector")

# Clang static analysis flags
set(ANALYSER_FLAGS
    "-Wthread-safety"
    "-Wthread-safety-analysis"
    "-Wthread-safety-negative"
)

add_compile_options(${ANALYSER_FLAGS})

add_subdirectory(handles)
add_subdirectory(analyser)
add_subdirectory(uda)
add_subdirectory(userland)
