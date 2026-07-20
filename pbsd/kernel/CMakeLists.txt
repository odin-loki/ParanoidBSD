# Freestanding dual-link proof (hosted build of the C ABI surface for CI).
# On HARDENEDBSD, also build via sys/modules/pbsd_cxx_hello/Makefile.

add_library(pbsd_cxx_hello_abi STATIC
    pbsd_cxx_hello.cpp
)
target_compile_options(pbsd_cxx_hello_abi PRIVATE -fno-exceptions -fno-rtti)
target_compile_features(pbsd_cxx_hello_abi PRIVATE cxx_std_23)
target_include_directories(pbsd_cxx_hello_abi PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Wave 4 dual-link header surface (C ABI; BMI from pbsd_kernel_shim at link time).
add_library(pbsd_kernel_dual_link INTERFACE)
target_include_directories(pbsd_kernel_dual_link INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/shims)
