export module pbsd.kde.kwin.effects.registry;

import pbsd.core;

/// Wave 3 pass 3 — built-in KWin effect plugin IDs (from */plugins/*/main.cpp).
export namespace pbsd::kde::kwin::effects::registry {

inline constexpr const char kBlur[] = "blur";
inline constexpr const char kSlide[] = "slide";
inline constexpr const char kMagicLamp[] = "magiclamp";
inline constexpr const char kOverview[] = "overview";
inline constexpr const char kZoom[] = "zoom";
inline constexpr const char kHideCursor[] = "hidecursor";
inline constexpr const char kKscreen[] = "kscreen";
inline constexpr const char kShowPaint[] = "showpaint";
inline constexpr const char kSlideBack[] = "slideback";
inline constexpr const char kMouseClick[] = "mouseclick";
inline constexpr const char kTrackMouse[] = "trackmouse";
inline constexpr const char kDimInactive[] = "diminactive";
inline constexpr const char kTouchPoints[] = "touchpoints";
inline constexpr const char kOutputLocator[] = "outputlocator";
inline constexpr const char kScreenEdge[] = "screenedge";
inline constexpr const char kThumbnailAside[] = "thumbnailaside";
inline constexpr const char kHighlightWindow[] = "highlightwindow";
inline constexpr const char kColorBlindnessCorrection[] = "colorblindnesscorrection";
inline constexpr const char kMouseMark[] = "mousemark";
inline constexpr const char kGlide[] = "glide";
inline constexpr const char kSheet[] = "sheet";
inline constexpr const char kInvert[] = "invert";
inline constexpr const char kBlendChanges[] = "blendchanges";
inline constexpr const char kTilesEditor[] = "tileseditor";
inline constexpr const char kFallApart[] = "fallapart";
inline constexpr const char kMagnifier[] = "magnifier";
inline constexpr const char kShowFps[] = "showfps";
inline constexpr const char kSystemBell[] = "systembell";
inline constexpr const char kColorPicker[] = "colorpicker";
inline constexpr const char kWindowView[] = "windowview";
inline constexpr const char kSlidingPopups[] = "slidingpopups";
inline constexpr const char kWobblyWindows[] = "wobblywindows";
inline constexpr const char kScreenTransform[] = "screentransform";
inline constexpr const char kShowCompositing[] = "showcompositing";
inline constexpr const char kStartupFeedback[] = "startupfeedback";
inline constexpr const char kShakeCursor[] = "shakecursor";
inline constexpr const char kBounceKeys[] = "bouncekeys";
inline constexpr const char kStickyKeys[] = "stickykeys";
inline constexpr const char kSlowKeys[] = "slowkeys";
inline constexpr const char kMouseKeys[] = "mousekeys";
inline constexpr const char kKeyNotification[] = "keynotification";
inline constexpr const char kButtonRebinds[] = "buttonrebinds";
inline constexpr const char kTouchpadShortcuts[] = "touchpadshortcuts";
inline constexpr const char kNightLight[] = "nightlight";
inline constexpr const char kScreenshot[] = "screenshot";
inline constexpr const char kGameController[] = "gamecontroller";
inline constexpr const char kKrunnerIntegration[] = "krunner-integration";
inline constexpr const char kEis[] = "eis";
inline constexpr const char kScreencast[] = "screencast";
inline constexpr const char kQpa[] = "qpa";
inline constexpr const char kPbsdAeroBlur[] = "pbsd_aero_blur";

inline constexpr unsigned kBuiltinEffectCount{50};

[[nodiscard]] inline bool is_pbsd_aero(const char* id) noexcept {
    return id != nullptr && id[0] == 'p' && id[1] == 'b' && id[2] == 's' && id[3] == 'd';
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/kwin/src/plugins/";
}

} // namespace pbsd::kde::kwin::effects::registry
