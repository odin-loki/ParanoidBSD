# Clang-based static analyser — requires LLVM/Clang development packages.
# Prefer a complete install: Ubuntu often ships ClangConfig for older versions
# whose libclangBasic.a is missing; importing those is a hard CMake error.
foreach(_ver IN ITEMS 18 17 16 15)
    set(_llvm_dir "/usr/lib/llvm-${_ver}/lib/cmake/llvm")
    set(_clang_dir "/usr/lib/cmake/clang-${_ver}")
    if(EXISTS "${_llvm_dir}/LLVMConfig.cmake"
       AND EXISTS "${_clang_dir}/ClangConfig.cmake"
       AND EXISTS "/usr/lib/llvm-${_ver}/lib/libclangBasic.a")
        set(LLVM_DIR "${_llvm_dir}")
        set(Clang_DIR "${_clang_dir}")
        break()
    endif()
endforeach()

find_package(LLVM CONFIG QUIET)
find_package(Clang CONFIG QUIET)

if(LLVM_FOUND AND Clang_FOUND)
    message(STATUS "LLVM ${LLVM_PACKAGE_VERSION} found — building analyser")
    add_library(pbsd_analyser MODULE src/ownership_check.cpp)
    target_include_directories(pbsd_analyser PRIVATE ${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS})
    target_link_libraries(pbsd_analyser PRIVATE clangAST clangASTMatchers clangTooling)
    target_compile_features(pbsd_analyser PRIVATE cxx_std_23)
else()
    message(WARNING "LLVM/Clang dev headers not found — analyser not built. Install libclang-*-dev llvm-*-dev.")
endif()
