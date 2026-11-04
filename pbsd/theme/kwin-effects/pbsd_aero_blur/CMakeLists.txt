# Wave 3 — PBSD Aero Blur KWin effect plugin scaffold
# Built from pbsd/theme when PBSD_BUILD_KWIN_PLUGINS=ON

set(PBSD_AERO_BLUR_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(PBSD_AERO_BLUR_SOURCES
    ${PBSD_AERO_BLUR_DIR}/contents/code/main.cpp
    ${PBSD_AERO_BLUR_DIR}/contents/code/pbsd_aero_blur.cpp
    ${PBSD_AERO_BLUR_DIR}/pbsd_aero_blur.qrc
)

kwin_add_builtin_effect(pbsd_aero_blur ${PBSD_AERO_BLUR_SOURCES})
target_include_directories(pbsd_aero_blur PRIVATE
    ${PBSD_AERO_BLUR_DIR}/contents/code)
target_link_libraries(pbsd_aero_blur PRIVATE KWin KF6::Config)

install(FILES
    metadata.json
    pbsd_aero_blur.qrc
    DESTINATION share/kwin/effects/pbsd_aero_blur
    OPTIONAL)

install(DIRECTORY contents/shaders/
    DESTINATION share/kwin/effects/pbsd_aero_blur/contents/shaders
    FILES_MATCHING
        PATTERN "*.vert"
        PATTERN "*.frag"
        PATTERN "*.json")

install(DIRECTORY contents/config/
    DESTINATION share/kwin/effects/pbsd_aero_blur/contents/config)
