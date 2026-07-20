# Wave 3 — KDE C++23 module stubs (theme constants + KWin bridge + KF conversions)
set(PBSD_KDE_CXX -fno-exceptions -fno-rtti)

set(PBSD_KDE_KWIN_LOGGING_MODULES
    kwin/logging.common.cppm
    kwin/backends.logging.cppm
    kwin/backends/drm/drm_backend.logging.cppm
    kwin/backends/libinput/libinput_backend.logging.cppm
    kwin/backends/virtual.logging.cppm
    kwin/core.logging.cppm
    kwin/decorations.logging.cppm
    kwin/drm.logging.cppm
    kwin/effect.logging.cppm
    kwin/libinput.logging.cppm
    kwin/scene.logging.cppm
    kwin/scripting.logging.cppm
    kwin/tabbox.logging.cppm
    kwin/virtual.logging.cppm
    kwin/wayland.logging.cppm
    kwin/x11.logging.cppm
)

set(PBSD_KDE_KWIN_EFFECT_MODULES
    kwin/effects/blendchanges.cppm
    kwin/effects/blur.cppm
    kwin/effects/dialogparent.cppm
    kwin/effects/bouncekeys.cppm
    kwin/effects/buttonrebinds.cppm
    kwin/effects/colorblindnesscorrection.cppm
    kwin/effects/colorpicker.cppm
    kwin/effects/diminactive.cppm
    kwin/effects/eis.cppm
    kwin/effects/fade.cppm
    kwin/effects/fallapart.cppm
    kwin/effects/gamecontroller.cppm
    kwin/effects/glide.cppm
    kwin/effects/hidecursor.cppm
    kwin/effects/highlightwindow.cppm
    kwin/effects/invert.cppm
    kwin/effects/keynotification.cppm
    kwin/effects/krunner_integration.cppm
    kwin/effects/kscreen.cppm
    kwin/effects/magiclamp.cppm
    kwin/effects/magnifier.cppm
    kwin/effects/mouseclick.cppm
    kwin/effects/mousekeys.cppm
    kwin/effects/mousemark.cppm
    kwin/effects/nightlight.cppm
    kwin/effects/outputlocator.cppm
    kwin/effects/overview.cppm
    kwin/effects/qpa.cppm
    kwin/effects/screencast.cppm
    kwin/effects/screenedge.cppm
    kwin/effects/screenshot.cppm
    kwin/effects/screentransform.cppm
    kwin/effects/shakecursor.cppm
    kwin/effects/sheet.cppm
    kwin/effects/showcompositing.cppm
    kwin/effects/showfps.cppm
    kwin/effects/showpaint.cppm
    kwin/effects/slide.cppm
    kwin/effects/slideback.cppm
    kwin/effects/slidingpopups.cppm
    kwin/effects/slowkeys.cppm
    kwin/effects/startupfeedback.cppm
    kwin/effects/stickykeys.cppm
    kwin/effects/systembell.cppm
    kwin/effects/thumbnailaside.cppm
    kwin/effects/tileseditor.cppm
    kwin/effects/touchpadshortcuts.cppm
    kwin/effects/touchpoints.cppm
    kwin/effects/trackmouse.cppm
    kwin/effects/windowview.cppm
    kwin/effects/wobblywindows.cppm
    kwin/effects/zoom.cppm
)

set(PBSD_KDE_FRAMEWORKS_MODULES
    frameworks/kconfig.gui_static.cppm
    frameworks/kconfig.inifile.cppm
    frameworks/kconfig.kauthorized.cppm
    frameworks/kconfig.kconfigbase.cppm
    frameworks/kconfig.kconfigcompiler.cppm
    frameworks/kconfig.kconfiggroupgui.cppm
    frameworks/kconfig.kconfiggui.cppm
    frameworks/kconfig.kconfigpropertymap.cppm
    frameworks/kconfig.kconfigskeleton.cppm
    frameworks/kconfig.kconfigwatcher.cppm
    frameworks/kconfig.kconfparameters.cppm
    frameworks/kconfig.kdesktopfileaction.cppm
    frameworks/kconfig.kemailsettings.cppm
    frameworks/kconfig.kreadconfig.cppm
    frameworks/kconfig.ksharedconfig.cppm
    frameworks/kconfig.kshortcut.cppm
    frameworks/kconfig.kstandardactions.cppm
    frameworks/kconfig.kstandardshortcutwatcher.cppm
    frameworks/kconfig.kwindowstatesaver.cppm
    frameworks/kconfig.kwindowstatesaverquick.cppm
    frameworks/kconfig.kwriteconfig.cppm
    frameworks/kconfig.types.cppm
    frameworks/kconfig.types_qml.cppm
    frameworks/kcoreaddons.filesystemtype.cppm
    frameworks/kcoreaddons.formats.cppm
    frameworks/kcoreaddons.kaboutdata.cppm
    frameworks/kcoreaddons.kautosavefile.cppm
    frameworks/kcoreaddons.kbackup.cppm
    frameworks/kcoreaddons.kcompositejob.cppm
    frameworks/kcoreaddons.kcoreaddons.cppm
    frameworks/kcoreaddons.kcoreaddonsplugin.cppm
    frameworks/kcoreaddons.kcrash.cppm
    frameworks/kcoreaddons.kdatetime.cppm
    frameworks/kcoreaddons.kdelibs4configmigrator.cppm
    frameworks/kcoreaddons.kemoticonsparser.cppm
    frameworks/kcoreaddons.kfileutils.cppm
    frameworks/kcoreaddons.kformat.cppm
    frameworks/kcoreaddons.kjob.cppm
    frameworks/kcoreaddons.kjobtrackerinterface.cppm
    frameworks/kcoreaddons.kjobuidelegate.cppm
    frameworks/kcoreaddons.kjsonutils.cppm
    frameworks/kcoreaddons.klibexec.cppm
    frameworks/kcoreaddons.klistopenfiles_unix.cppm
    frameworks/kcoreaddons.klocalizedstring.cppm
    frameworks/kcoreaddons.kmacroexpander_unix.cppm
    frameworks/kcoreaddons.kmemoryinfo.cppm
    frameworks/kcoreaddons.knetworkmounts.cppm
    frameworks/kcoreaddons.kpluginfactory.cppm
    frameworks/kcoreaddons.kprocesslist_unix.cppm
    frameworks/kcoreaddons.kprocesslist_unix_procstat.cppm
    frameworks/kcoreaddons.ksandbox.cppm
    frameworks/kcoreaddons.ksdclock.cppm
    frameworks/kcoreaddons.kshell.cppm
    frameworks/kcoreaddons.ksignalhandler.cppm
    frameworks/kcoreaddons.kstaticplugin.cppm
    frameworks/kcoreaddons.kstaticpluginhelpers.cppm
    frameworks/kcoreaddons.kstringhandler.cppm
    frameworks/kcoreaddons.ktexttohtml.cppm
    frameworks/kcoreaddons.kurlmimedata.cppm
    frameworks/kcoreaddons.kuserproxy.cppm
    frameworks/kcoreaddons.processlist.cppm
    frameworks/kcoreaddons.procstat.cppm
    frameworks/kcoreaddons.random.cppm
    frameworks/kcoreaddons.runtimeplatform.cppm
    frameworks/kdecoration.bridge.cppm
    frameworks/kdecoration.decorationbridge_private.cppm
    frameworks/kio.askuseractioninterface.cppm
    frameworks/kio.connectionserver.cppm
    frameworks/kio.copyjob.cppm
    frameworks/kio.dndpopupmenuplugin.cppm
    frameworks/kio.emptytrashjob.cppm
    frameworks/kio.jobtracker.cppm
    frameworks/kio.jobuidelegateextension.cppm
    frameworks/kio.jobuidelegatefactory.cppm
    frameworks/kio.joburlcache.cppm
    frameworks/kio.kabstractfileitemactionplugin.cppm
    frameworks/kio.kfilewidgetdocktitlebar.cppm
    frameworks/kio.kioglobal_unix.cppm
    frameworks/kio.koverlayiconplugin.cppm
    frameworks/kio.kpasswdserver.cppm
    frameworks/kio.kpasswdserverloop.cppm
    frameworks/kio.kpreviewwidgetbase.cppm
    frameworks/kio.kurlauthorized.cppm
    frameworks/kio.kurlnavigatorpathselectoreventfilter.cppm
    frameworks/kio.metadata.cppm
    frameworks/kio.openorexecutefileinterface.cppm
    frameworks/kio.openwithhandlerinterface.cppm
    frameworks/kio.protocols.cppm
    frameworks/kio.remotedirnotify.cppm
    frameworks/kio.remotedirnotifymodule.cppm
    frameworks/kio.specialjob.cppm
    frameworks/kio.statjob.cppm
    frameworks/kio.workerfactory.cppm
    frameworks/kio.workerthread.cppm
    frameworks/kguiaddons.kcolorscheme.cppm
    frameworks/kirigami.units.cppm
    frameworks/kwidgets.kseparator.cppm
    frameworks/kwidgets.kswitch.cppm
    frameworks/kwidgets.lineedit.cppm
    frameworks/kwidgets.messagewidget.cppm
    frameworks/kwindowsystem.kusertimestamp.cppm
    frameworks/kwindowsystem.kwindoweffects.cppm
    frameworks/kwindowsystem.kwindowsystem.cppm
    frameworks/kwindowsystem.netwm.cppm
    frameworks/kwindowsystem.plugininterface.cppm
    frameworks/kwindowsystem.startupinfo.cppm
    frameworks/kwindowsystem.wayland_plugin.cppm
    frameworks/kwindowsystem.waylandxdgactivationv1.cppm
    frameworks/kwindowsystem.xcb_plugin.cppm
    frameworks/kxmlgui.kaboutapplicationlistview.cppm
    frameworks/kxmlgui.kbugreport.cppm
    frameworks/kxmlgui.kmainwindow.cppm
    frameworks/layershell.interfaces_shell.cppm
    frameworks/layershell.layershellintegration.cppm
    frameworks/layershell.bridge.cppm
    frameworks/layershell.qwaylandlayershell.cppm
    frameworks/layershell.qwaylandlayersurface.cppm
    frameworks/layershell.qwaylandxdgactivationv1.cppm
    frameworks/layershell.shell.cppm
    frameworks/layershell.interfaces_region.cppm
    frameworks/layershell.interfaces_output.cppm
    frameworks/layershell.qwaylandlayershellintegrationplugin.cppm
    frameworks/layershell.interfaces_window.cppm
    frameworks/plasma.applet_packagestructure.cppm
    frameworks/plasma.containmentactions.cppm
    frameworks/plasma.containmentactions_packagestructure.cppm
    frameworks/plasma.corebindingsplugin.cppm
    frameworks/plasma.packagestructure_wallpaper.cppm
    frameworks/plasma.plasma.cppm
    frameworks/plasma.plasmaquick_appletcontext.cppm
    frameworks/plasma.plasmaquick_utils.cppm
    frameworks/plasma.utils.cppm
    frameworks/kwindoweffects.blur.cppm
    frameworks/plasma.layer_surface.cppm
    frameworks/wayland.bridge.cppm
    frameworks/plasma.bridge.cppm
)

set(PBSD_KDE_PLASMA_MODULES
    plasma.aero.cppm
    plasma.baloo.baloodata.cppm
    plasma.baloo.cppm
    plasma.containment.cppm
    plasma.cursortheme.cppm
    plasma.desktop.logging.cppm
    plasma.digitalclock.cppm
    plasma.emojier.category.cppm
    plasma.emojier.cppm
    plasma.folder.applauncher.cppm
    plasma.gamecontroller.cppm
    plasma.globalpaths.cppm
    plasma.kded.cppm
    plasma.keyboard.cppm
    plasma.keyboard.layoutnames.cppm
    plasma.kicker.cppm
    plasma.kimpanel.ibus.cppm
    plasma.knetattach.cppm
    plasma.libinputcommon.cppm
    plasma.lookandfeel.cppm
    plasma.mediacontroller.cppm
    plasma.menuhelper.cppm
    plasma.mouse.kapplymousetheme.cppm
    plasma.mouse.logging.cppm
    plasma.notifications.cppm
    plasma.packagestructure.cppm
    plasma.panel.cppm
    plasma.qtquicksettings.cppm
    plasma.quicklaunch.cppm
    plasma.runners.abstractjob.cppm
    plasma.runners.zypperrpmjob.cppm
    plasma.shell.cppm
    plasma.showdesktop.cppm
    plasma.startmenu.cppm
    plasma.systemsettings.cppm
    plasma.systray.cppm
    plasma.tablet.cppm
    plasma.taskbar.cppm
    plasma.taskmanager.cppm
    plasma.volume.cppm
    plasma.tastenbrett.application.cppm
    plasma.tastenbrett.geometry.cppm
    plasma.tastenbrett.outline.cppm
    plasma.tastenbrett.row.cppm
    plasma.tastenbrett.section.cppm
    plasma.tastenbrett.shape.cppm
    plasma.touchpad.logging.cppm
    plasma.touchscreen.cppm
    plasma.touchscreen.moduledata.cppm
    plasma.wheelinterceptor.cppm
    plasma.workspace.logging.cppm
    plasma.dialogs.cppm
    plasma.widgets.cppm
    plasma.colors.cppm
    plasma.systemlinks.cppm
    plasma.systemlinks.glue.cppm
    plasma.poweractions.cppm
    plasma.quickaccess.cppm
    plasma.wayland.glue.cppm
    plasma.lookandfeel.glue.cppm
    plasma.containment.glue.cppm
    plasma.decoration.glue.cppm
    plasma.layershell.glue.cppm
    plasma.taskbar.glue.cppm
    plasma.notifications.glue.cppm
    plasma.startmenu.glue.cppm
    plasma.systray.glue.cppm
    plasma.volume.glue.cppm
    plasma.digitalclock.glue.cppm
    plasma.quicklaunch.glue.cppm
    plasma.poweractions.glue.cppm
    plasma.shell.glue.cppm
    plasma.mediacontroller.glue.cppm
    plasma.keyboard.glue.cppm
    plasma.kicker.glue.cppm
    plasma.showdesktop.glue.cppm
    plasma.gamecontroller.glue.cppm
    plasma.panel.glue.cppm
    plasma.colors.glue.cppm
    plasma.dialogs.glue.cppm
    plasma.widgets.glue.cppm
    plasma.kded.glue.cppm
    plasma.systemsettings.glue.cppm
    plasma.globalpaths.glue.cppm
    plasma.cursortheme.glue.cppm
    plasma.baloo.glue.cppm
    plasma.emojier.glue.cppm
    plasma.quickaccess.glue.cppm
    plasma.menuhelper.glue.cppm
    plasma.libinputcommon.glue.cppm
    plasma.qtquicksettings.glue.cppm
    plasma.packagestructure.glue.cppm
    plasma.tastenbrett.glue.cppm
    plasma.touchscreen.glue.cppm
    plasma.tablet.glue.cppm
    plasma.taskmanager.glue.cppm
    plasma.wheelinterceptor.glue.cppm
    plasma.knetattach.glue.cppm
    plasma.folder.applauncher.glue.cppm
    plasma.kimpanel.ibus.glue.cppm
    plasma.mouse.kapplymousetheme.glue.cppm
    plasma.clipboard.cppm
    plasma.brightness.cppm
    plasma.calendar.cppm
    plasma.containmentactions.cppm
    plasma.screenlock.cppm
    plasma.accessibility.cppm
    plasma.clipboard.glue.cppm
    plasma.brightness.glue.cppm
    plasma.calendar.glue.cppm
    plasma.containmentactions.glue.cppm
    plasma.screenlock.glue.cppm
    plasma.accessibility.glue.cppm
    plasma.runners.shell.glue.cppm
    plasma.kcms.display.glue.cppm
    plasma.runners.shell.cppm
    plasma.kcms.display.cppm
    plasma.runners.calculator.cppm
    plasma.runners.calculator.glue.cppm
    plasma.runners.bookmarks.cppm
    plasma.runners.bookmarks.glue.cppm
    plasma.runners.locations.cppm
    plasma.runners.locations.glue.cppm
    plasma.tastenbrett.application.glue.cppm
    plasma.tastenbrett.geometry.glue.cppm
    plasma.tastenbrett.outline.glue.cppm
    plasma.tastenbrett.row.glue.cppm
    plasma.tastenbrett.shape.glue.cppm
    plasma.touchscreen.moduledata.glue.cppm
    plasma.baloo.baloodata.glue.cppm
    plasma.emojier.category.glue.cppm
    plasma.keyboard.layoutnames.glue.cppm
    plasma.runners.abstractjob.glue.cppm
    plasma.runners.zypperrpmjob.glue.cppm
)

# Foundation plasma modules — separate targets so cross-library imports get PCM
# paths in consumer modmaps (avoids parallel-build races in large FILE_SET bundles).
set(PBSD_KDE_PLASMA_FOUNDATION
    plasma.aero.cppm
    plasma.panel.cppm
)

set(PBSD_KDE_PLASMA_MODULES ${PBSD_KDE_PLASMA_MODULES})
list(REMOVE_ITEM PBSD_KDE_PLASMA_MODULES ${PBSD_KDE_PLASMA_FOUNDATION})

if(NOT TARGET pbsd_kde_plasma_aero_module)
add_library(pbsd_kde_plasma_aero_module)
target_sources(pbsd_kde_plasma_aero_module PUBLIC FILE_SET CXX_MODULES FILES plasma.aero.cppm)
target_link_libraries(pbsd_kde_plasma_aero_module PUBLIC pbsd_core)
target_compile_features(pbsd_kde_plasma_aero_module PUBLIC cxx_std_23)
target_compile_options(pbsd_kde_plasma_aero_module PRIVATE ${PBSD_KDE_CXX})
endif()

if(NOT TARGET pbsd_kde_plasma_panel_module)
add_library(pbsd_kde_plasma_panel_module)
target_sources(pbsd_kde_plasma_panel_module PUBLIC FILE_SET CXX_MODULES FILES plasma.panel.cppm)
target_link_libraries(pbsd_kde_plasma_panel_module PUBLIC pbsd_core pbsd_kde_plasma_aero_module)
target_compile_features(pbsd_kde_plasma_panel_module PUBLIC cxx_std_23)
target_compile_options(pbsd_kde_plasma_panel_module PRIVATE ${PBSD_KDE_CXX})
endif()

set(PBSD_KDE_KWIN_MODULES
    kwin.pbsd_aero.cppm
    kwin.decorations.cppm
    kwin/backends/drm/drm_abstract_output.cppm
    kwin/backends/drm/drm_output.cppm
    kwin/backgroundeffectitem.cppm
    kwin/blur.cppm
    kwin/colorpipelinestage.cppm
    kwin/compositing.cppm
    kwin/core/colorlut3d.cppm
    kwin/desktopeffectsdata.cppm
    kwin/drm_blob.cppm
    kwin/effects.registry.cppm
    kwin/graphicsbufferallocator.cppm
    kwin/helpers/kwindowprop.cppm
    kwin/input/sp_keyboard.cppm
    kwin/input_event.cppm
    kwin/inputbackend.cppm
    kwin/inputpanelv1integration.cppm
    kwin/itemrenderer.cppm
    kwin/kcms/screenedges_config.cppm
    kwin/kcms/tabbox_data.cppm
    kwin/killwindow.cppm
    kwin/kwinscriptsdata.cppm
    kwin/lidswitchtracker.cppm
    kwin/mousebuttons.cppm
    kwin/opengl/abstract_opengl_context_attribute_builder.cppm
    kwin/opengl/eglimagetexture.cppm
    kwin/outputconfiguration.cppm
    kwin/plugin.cppm
    kwin/plugins/aurorae.cppm
    kwin/plugins/blur_config.cppm
    kwin/plugins/colorpickerlayer.cppm
    kwin/plugins/eisplugin.cppm
    kwin/plugins/filteredsceneview.cppm
    kwin/plugins/glide_config.cppm
    kwin/plugins/kpackage_decoration.cppm
    kwin/plugins/kpackage_effect.cppm
    kwin/plugins/kpackage_windowswitcher.cppm
    kwin/plugins/magiclamp_config.cppm
    kwin/plugins/offscreensurface.cppm
    kwin/plugins/platformcursor.cppm
    kwin/plugins/qpainterbackend.cppm
    kwin/plugins/screenshotlayer.cppm
    kwin/plugins/scripts.cppm
    kwin/plugins/slide_config.cppm
    kwin/plugins/zoom_focustracker.cppm
    kwin/renderjournal.cppm
    kwin/renderloopdrivenqanimationdriver.cppm
    kwin/rootinfo_filter.cppm
    kwin/rootitem.cppm
    kwin/scene/dndiconitem.cppm
    kwin/screencast.cppm
    kwin/scripting/dbuscall.cppm
    kwin/surfaceitem_internal.cppm
    kwin/syncalarmx11filter.cppm
    kwin/textinput.cppm
    kwin/utils/executable_path_sysctl.cppm
    kwin/utils/realtime.cppm
    kwin/vsyncmonitor.cppm
    kwin/wayland/abstract_data_source.cppm
    kwin/wayland/abstract_drop_handler.cppm
    kwin/wayland/fixes.cppm
    kwin/wayland/keyboard_shortcuts_inhibit.cppm
    kwin/wayland/pointerwarp_v1.cppm
    kwin/wayland/presentation.cppm
    kwin/wayland/region.cppm
    kwin/wayland/server_decoration.cppm
    kwin/wayland/textinput_v3.cppm
    kwin/wayland/xdgtopleveltag_v1.cppm
    kwin/waylandshellintegration.cppm
    kwin/window_property_notify_x11_filter.cppm
    kwin/x11eventfilter.cppm
    kwin/xwayland/drag.cppm
    kwin/xxpipv1integration.cppm
    kwin/aero.glow.cppm
    kwin/aero.panel.cppm
    kwin/aero.startmenu.cppm
    kwin/effects/pbsd_aero_blur.cppm
    kwin/wayland/xdg_shell.cppm
    kwin/wayland/layer_shell.cppm
    kwin/wayland/xdg_decoration.cppm
    kwin/wayland/blur_contrast.cppm
    kwin/wayland/subcompositor.cppm
    kwin/wayland/viewporter.cppm
    kwin/wayland/fractional_scale.cppm
    kwin/compositing.theme.cppm
    kwin/wayland/cursor_shape.cppm
    kwin/wayland/content_type.cppm
    kwin/wayland/alpha_modifier.cppm
    kwin/aero.dialog.cppm
    kwin/wayland/idle_inhibit.cppm
    kwin/wayland/tablet_v2.cppm
    kwin/wayland/pointer_constraints.cppm
    kwin/wayland/data_device.cppm
    kwin/wayland/seat.cppm
    kwin/wayland/relative_pointer.cppm
    kwin/wayland/pointer_gestures.cppm
    kwin/wayland/primary_selection.cppm
    kwin/wayland/linux_dmabuf.cppm
    kwin/wayland/single_pixel_buffer.cppm
    kwin/wayland/tearing_control.cppm
    kwin/wayland/drm.cppm
)

if(NOT TARGET pbsd_kde_kwin_logging)
add_library(pbsd_kde_kwin_logging)
target_sources(pbsd_kde_kwin_logging PUBLIC FILE_SET CXX_MODULES FILES ${PBSD_KDE_KWIN_LOGGING_MODULES})
target_link_libraries(pbsd_kde_kwin_logging PUBLIC pbsd_core)
target_compile_features(pbsd_kde_kwin_logging PUBLIC cxx_std_23)
target_compile_options(pbsd_kde_kwin_logging PRIVATE ${PBSD_KDE_CXX})
endif()

if(NOT TARGET pbsd_kde_kwin_effects)
add_library(pbsd_kde_kwin_effects)
target_sources(pbsd_kde_kwin_effects PUBLIC FILE_SET CXX_MODULES FILES ${PBSD_KDE_KWIN_EFFECT_MODULES})
target_link_libraries(pbsd_kde_kwin_effects PUBLIC pbsd_kde_kwin_logging)
target_compile_features(pbsd_kde_kwin_effects PUBLIC cxx_std_23)
target_compile_options(pbsd_kde_kwin_effects PRIVATE ${PBSD_KDE_CXX})
endif()

if(NOT TARGET pbsd_kde_plasma_aero)
add_library(pbsd_kde_plasma_aero)
target_sources(pbsd_kde_plasma_aero PUBLIC FILE_SET CXX_MODULES FILES ${PBSD_KDE_PLASMA_MODULES})
target_link_libraries(pbsd_kde_plasma_aero PUBLIC pbsd_core pbsd_kde_kwin_logging
    pbsd_kde_plasma_aero_module pbsd_kde_plasma_panel_module)
target_compile_features(pbsd_kde_plasma_aero PUBLIC cxx_std_23)
target_compile_options(pbsd_kde_plasma_aero PRIVATE ${PBSD_KDE_CXX})
endif()

if(NOT TARGET pbsd_kde_kwin_aero)
add_library(pbsd_kde_kwin_aero)
target_sources(pbsd_kde_kwin_aero PUBLIC FILE_SET CXX_MODULES FILES ${PBSD_KDE_KWIN_MODULES})
target_link_libraries(pbsd_kde_kwin_aero PUBLIC pbsd_kde_plasma_aero pbsd_kde_plasma_aero_module
    pbsd_kde_kwin_effects pbsd_handles pbsd_kde_kwin_logging)
target_compile_features(pbsd_kde_kwin_aero PUBLIC cxx_std_23)
target_compile_options(pbsd_kde_kwin_aero PRIVATE ${PBSD_KDE_CXX})
endif()

if(NOT TARGET pbsd_kde_frameworks)
add_library(pbsd_kde_frameworks)
target_sources(pbsd_kde_frameworks PUBLIC FILE_SET CXX_MODULES FILES ${PBSD_KDE_FRAMEWORKS_MODULES})
target_link_libraries(pbsd_kde_frameworks PUBLIC pbsd_core pbsd_kde_plasma_aero
    pbsd_kde_plasma_aero_module pbsd_kde_plasma_panel_module)
target_compile_features(pbsd_kde_frameworks PUBLIC cxx_std_23)
target_compile_options(pbsd_kde_frameworks PRIVATE ${PBSD_KDE_CXX})

if(CMAKE_SYSTEM_NAME STREQUAL "FreeBSD")
    target_link_libraries(pbsd_kde_frameworks PRIVATE procstat)
endif()
endif()

if(NOT TARGET pbsd_kde)
add_library(pbsd_kde INTERFACE)
target_link_libraries(pbsd_kde INTERFACE
    pbsd_kde_plasma_aero_module
    pbsd_kde_plasma_panel_module
    pbsd_kde_plasma_aero
    pbsd_kde_kwin_aero
    pbsd_kde_kwin_effects
    pbsd_kde_kwin_logging
    pbsd_kde_frameworks)
endif()

set(PBSD_KDE_MODULE_FILES
    ${PBSD_KDE_PLASMA_FOUNDATION}
    ${PBSD_KDE_PLASMA_MODULES}
    ${PBSD_KDE_KWIN_MODULES}
    ${PBSD_KDE_KWIN_EFFECT_MODULES}
    ${PBSD_KDE_KWIN_LOGGING_MODULES}
    ${PBSD_KDE_FRAMEWORKS_MODULES})

install(FILES ${PBSD_KDE_MODULE_FILES}
    DESTINATION share/pbsd/kde/modules
    OPTIONAL)
