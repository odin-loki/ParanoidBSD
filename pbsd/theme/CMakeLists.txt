# Wave 3 — Aero theme package (data + C++23 Aero modules + install rules)



set(PBSD_THEME_ID pbsd-aero)

set(PBSD_THEME_SRC ${CMAKE_CURRENT_SOURCE_DIR})

set(PBSD_THEME_CXX -fno-exceptions -fno-rtti)



set(PBSD_THEME_PLASMA_AERO_MODULES

    plasma/aero.dwm_blur.cppm

    plasma/aero.glass.cppm

    plasma/aero.start_menu.cppm

    plasma/aero.taskbar.cppm

    plasma/aero.window_chrome.cppm

    plasma/aero.colors.cppm

    plasma/aero.dialogs.cppm

    plasma/aero.widgets.cppm

    plasma/aero.panel.cppm

    plasma/aero.glow.cppm

    plasma/aero.system_links.cppm

    plasma/aero.power_actions.cppm

    plasma/aero.notifications.cppm

    plasma/aero.compositing.cppm

    plasma/aero.accent.cppm

    plasma/aero.dialog.cppm

    plasma/aero.notification.cppm

    plasma/aero.icons.cppm

    plasma/aero.bifrost_tokens.cppm

    plasma/aero.kwin_bridge.cppm

    plasma/aero.layershell.cppm

    plasma/aero.presentation.cppm

    plasma/aero.shadows.cppm

    plasma/aero.typography.cppm

    plasma/aero.transitions.cppm

    plasma/aero.wallpaper.cppm

    plasma/aero.focus.cppm

    plasma/aero.borders.cppm

    plasma/aero.cursors.cppm

    plasma/aero.tooltips.cppm

    plasma/aero.high_contrast.cppm

    plasma/aero.caption_buttons.cppm

    plasma/aero.decoration_borders.cppm

    plasma/aero.search_box.cppm

    plasma/aero.system_tray.cppm

    plasma/aero.contrast.cppm

    plasma/aero.quick_launch.cppm

    plasma/aero.user_tile.cppm

    plasma/aero.sidebar.cppm

    plasma/aero.frame_assets.cppm

    plasma/aero.favorites.cppm

    plasma/aero.clock_widget.cppm

    plasma/aero.media_controls.cppm

    plasma/aero.task_preview.cppm

    plasma/aero.window_snap.cppm

    plasma/aero.start_menu_search.cppm

    plasma/aero.alt_tab.cppm

    plasma/aero.battery_indicator.cppm

    plasma/aero.network_status.cppm

    plasma/aero.desktop_icons.cppm

    plasma/aero.jump_list.cppm

    plasma/aero.volume_mixer.cppm

    plasma/aero.brightness_slider.cppm

    plasma/aero.calendar_widget.cppm

    plasma/aero.clipboard_history.cppm

    plasma/aero.screen_lock.cppm

    plasma/aero.privacy_indicator.cppm

    plasma/aero.workspace_switcher.cppm

    plasma/aero.action_center.cppm

    plasma/aero.magnifier.cppm

    plasma/aero.screen_reader.cppm

    plasma/aero.immersive.cppm

    plasma/aero.ribbon.cppm

    plasma/aero.task_switcher.cppm

)



if(NOT TARGET pbsd_theme_plasma_aero)

add_library(pbsd_theme_plasma_aero)

target_sources(pbsd_theme_plasma_aero PUBLIC FILE_SET CXX_MODULES FILES ${PBSD_THEME_PLASMA_AERO_MODULES})

target_link_libraries(pbsd_theme_plasma_aero PUBLIC pbsd_core pbsd_kde_plasma_aero pbsd_kde_kwin_aero)

target_compile_features(pbsd_theme_plasma_aero PUBLIC cxx_std_23)

target_compile_options(pbsd_theme_plasma_aero PRIVATE ${PBSD_THEME_CXX})

endif()



if(NOT TARGET pbsd_theme)

add_library(pbsd_theme INTERFACE)

target_include_directories(pbsd_theme INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(pbsd_theme INTERFACE pbsd_theme_plasma_aero)

endif()



# --- Install Plasma desktop theme ------------------------------------------------

install(DIRECTORY plasma/

    DESTINATION share/plasma/desktoptheme/${PBSD_THEME_ID}

    PATTERN "decoration/DECORATION.md" EXCLUDE

    PATTERN "decoration/metadata.json" EXCLUDE

    PATTERN "decoration/config.json" EXCLUDE

    PATTERN "panel/start-menu.md" EXCLUDE)



install(FILES plasma/panel/start-menu.md

    DESTINATION share/doc/pbsd-theme/plasma/panel

    OPTIONAL)



install(FILES plasma/color-schemes/PBSDAero.colors

    DESTINATION share/color-schemes

    RENAME PBSDAero.colors)



# --- Install KWin effect package (metadata, config, shaders; code via doc path) -

install(DIRECTORY kwin-effects/pbsd_aero_blur/

    DESTINATION share/kwin/effects/pbsd_aero_blur

    PATTERN "contents/code/*" EXCLUDE

    PATTERN "CMakeLists.txt" EXCLUDE

    PATTERN "pbsd_aero_blur.qrc" EXCLUDE)



install(FILES kwin-effects/pbsd_aero_blur/pbsd_aero_blur.qrc

    DESTINATION share/kwin/effects/pbsd_aero_blur

    OPTIONAL)



# Ship effect sources for downstream KWin builds (not loaded at runtime)

install(DIRECTORY kwin-effects/pbsd_aero_blur/contents/code/

    DESTINATION share/doc/pbsd-theme/kwin-effects/pbsd_aero_blur/code

    FILES_MATCHING PATTERN "*.cpp" PATTERN "*.h")



install(FILES kwin-effects/pbsd_aero_blur/CMakeLists.txt

    DESTINATION share/doc/pbsd-theme/kwin-effects/pbsd_aero_blur

    OPTIONAL)



# --- Install KDecoration3 decoration assets --------------------------------------

install(FILES plasma/decoration/metadata.json

    DESTINATION share/kwin/decoration/pbsd_aero)



install(FILES plasma/decoration/config.json

    DESTINATION share/kwin/decoration/pbsd_aero/contents)



install(DIRECTORY plasma/decoration/

    DESTINATION share/kwin/decoration/pbsd_aero/contents

    FILES_MATCHING

        PATTERN "*.svg"

        PATTERN "DECORATION.md")



# --- Export package config for downstream CMake ----------------------------------

install(FILES README.md

    DESTINATION share/doc/pbsd-theme

    OPTIONAL)



install(FILES ${PBSD_THEME_PLASMA_AERO_MODULES}

    DESTINATION share/pbsd/theme/modules

    OPTIONAL)





# --- Optional: build KWin plugin when KWin tree + deps are present -------------

option(PBSD_BUILD_KWIN_PLUGINS "Build PBSD KWin effect plugins (requires KWin/KF6)" OFF)



if(PBSD_BUILD_KWIN_PLUGINS)

    find_package(ECM REQUIRED NO_MODULE)

    list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH})

    find_package(KWin REQUIRED)

    find_package(KF6 REQUIRED COMPONENTS Config)

    add_subdirectory(kwin-effects/pbsd_aero_blur)

endif()

