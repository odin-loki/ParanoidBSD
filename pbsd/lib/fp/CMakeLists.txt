# pbsd.fp builds and tests on its own. The top-level pbsd target is a
# ratchet over 149 modules that cannot compile on a Linux runner because they
# include FreeBSD private headers; this one has no such dependency - it needs
# <bit>, <cstdint> and <type_traits> and nothing else - so it is built and
# *run* rather than counted.
cmake_minimum_required(VERSION 3.28)
project(pbsd_fp CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_SCAN_FOR_MODULES ON)

add_library(pbsd_fp STATIC)
target_sources(pbsd_fp PUBLIC
  FILE_SET CXX_MODULES FILES pbsd.fp.cppm)

add_executable(pbsd_fp_test tests/fp_test.cpp)
target_link_libraries(pbsd_fp_test PRIVATE pbsd_fp)
# The test enumerates all 2^32 binary32 bit patterns. Unoptimised that is a
# minute; at -O2 it is a couple of seconds, and the thing under test is the
# module, which is compiled the same either way.
target_compile_options(pbsd_fp_test PRIVATE -O2)

enable_testing()
add_test(NAME pbsd_fp COMMAND pbsd_fp_test)
