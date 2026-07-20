# Clang-based static analyser — hosted (not freestanding)
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
    message(STATUS "LLVM ${LLVM_PACKAGE_VERSION} — building pbsd_analyser plugins")

    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/pbsd_analyser_ownership.dir")
    file(MAKE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/pbsd_analyser_isr.dir")

    add_library(pbsd_analyser_ownership MODULE ownership_check.cpp)
    set_source_files_properties(ownership_check.cpp PROPERTIES CXX_SCAN_FOR_MODULES OFF)
    target_include_directories(pbsd_analyser_ownership PRIVATE
        ${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS})
    target_link_libraries(pbsd_analyser_ownership PRIVATE
        clangAST clangTooling)
    target_compile_features(pbsd_analyser_ownership PRIVATE cxx_std_23)
    set_property(TARGET pbsd_analyser_ownership PROPERTY JOB_POOL_COMPILE analyser_serial)

    add_library(pbsd_analyser_isr MODULE isr_context_check.cpp)
    set_source_files_properties(isr_context_check.cpp PROPERTIES CXX_SCAN_FOR_MODULES OFF)
    target_include_directories(pbsd_analyser_isr PRIVATE
        ${LLVM_INCLUDE_DIRS} ${CLANG_INCLUDE_DIRS})
    target_link_libraries(pbsd_analyser_isr PRIVATE
        clangAST clangTooling)
    target_compile_features(pbsd_analyser_isr PRIVATE cxx_std_23)
    set_property(TARGET pbsd_analyser_isr PROPERTY JOB_POOL_COMPILE analyser_serial)

    set_property(GLOBAL PROPERTY JOB_POOLS analyser_serial=1)

    add_custom_target(pbsd_analyser ALL
        DEPENDS pbsd_analyser_ownership pbsd_analyser_isr)
else()
    message(WARNING "LLVM/Clang dev packages missing — analyser skipped")
endif()
