export module pbsd.kde.plasma.poweractions.glue;

import pbsd.core;
import pbsd.kde.plasma.aero;
import pbsd.kde.plasma.poweractions;
import pbsd.kde.plasma.startmenu;

/// Burst 12 — Start menu power tiles ↔ Aero action chrome glue.
/// Upstream: pbsd/theme/plasma/panel/layout.json
export namespace pbsd::kde::plasma::poweractions::glue {

struct PowerTile {
    poweractions::Action action{};
    const char* svg{nullptr};
    poweractions::Severity severity{poweractions::Severity::Normal};
};

[[nodiscard]] inline const char* svg_for(poweractions::Action a) noexcept {
    switch (a) {
    case poweractions::Action::Lock:
        return poweractions::kLockSvg;
    case poweractions::Action::Sleep:
        return poweractions::kSleepSvg;
    case poweractions::Action::Restart:
        return poweractions::kRestartSvg;
    case poweractions::Action::Logoff:
        return poweractions::kLogoffSvg;
    case poweractions::Action::Shutdown:
        return poweractions::kShutdownSvg;
    case poweractions::Action::PowerButton:
        return poweractions::kPowerButtonSvg;
    }
    return poweractions::kPowerButtonSvg;
}

[[nodiscard]] inline PowerTile tile_for(poweractions::Action a) noexcept {
    PowerTile t{};
    t.action = a;
    t.svg = svg_for(a);
    t.severity = a == poweractions::Action::Shutdown
        ? poweractions::Severity::Destructive
        : poweractions::Severity::Normal;
    return t;
}

[[nodiscard]] inline float opacity_for(poweractions::Action a) noexcept {
    return a == poweractions::Action::Shutdown ? 0.92f : 0.78f;
}

[[nodiscard]] inline Status validate_action_count(unsigned count) noexcept {
    if (count == 0 || count > poweractions::kCount) {
        return Status::Invalid;
    }
    return Status::Ok;
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return startmenu::kNotesPath;
}

} // namespace pbsd::kde::plasma::poweractions::glue
