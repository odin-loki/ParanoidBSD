export module pbsd.kde.plasma.poweractions;

import pbsd.core;

/// Wave 3 pass 6 — Start menu power action buttons (layout.json powerActions).
/// Upstream: pbsd/theme/plasma/panel/layout.json
export namespace pbsd::kde::plasma::poweractions {

    enum class Action : unsigned char { Lock, Sleep, Restart, Logoff, Shutdown, PowerButton };
    inline constexpr unsigned kCount{6};
    inline constexpr const char kLockSvg[] = "plasma/panel/lock.svg";
    inline constexpr const char kSleepSvg[] = "plasma/panel/sleep.svg";
    inline constexpr const char kRestartSvg[] = "plasma/panel/restart.svg";
    inline constexpr const char kLogoffSvg[] = "plasma/panel/logoff.svg";
    inline constexpr const char kShutdownSvg[] = "plasma/panel/shutdown.svg";
    inline constexpr const char kPowerButtonSvg[] = "plasma/panel/power-button.svg";
    enum class Severity : unsigned char { Normal, Caution, Destructive };
    inline constexpr Severity kShutdownSeverity{Severity::Destructive};

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "pbsd/theme/plasma/panel/layout.json";
}

} // namespace pbsd::kde::plasma::poweractions
