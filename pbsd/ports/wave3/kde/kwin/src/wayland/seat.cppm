export module pbsd.port.wave3.kde.kwin.src.wayland.seat;

module;
extern "C" {
// Legacy TU symbols — hand-port from kde/kwin/src/wayland/seat.cpp
// void seat_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=kde/kwin/src/wayland/seat.cpp wave=wave3 loc=1343
export namespace pbsd::port::wave3::kde::kwin::src::wayland::seat {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave3::kde::kwin::src::wayland::seat
