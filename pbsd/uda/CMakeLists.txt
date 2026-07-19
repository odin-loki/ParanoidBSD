add_library(pbsd_uda STATIC
    src/descriptor_db.cpp
    src/block_engine.cpp
    src/net_engine.cpp
)
target_include_directories(pbsd_uda PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(pbsd_uda PRIVATE pbsd_handles)
target_compile_features(pbsd_uda PRIVATE cxx_std_23)
