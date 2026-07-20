export module pbsd.port.wave3.kde.kwin.src.core.session_noop;

module;
extern "C" {
// Legacy TU symbols — hand-port from kde/kwin/src/core/session_noop.cpp
// void session_noop_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=kde/kwin/src/core/session_noop.cpp wave=wave3 loc=74
export namespace pbsd::port::wave3::kde::kwin::src::core::session_noop {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave3::kde::kwin::src::core::session_noop
