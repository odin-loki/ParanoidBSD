export module pbsd.port.wave3.kde.kwin.src.wayland.quirks;

module;
// Header bridge — replace #include of kde/kwin/src/wayland/quirks.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=kde/kwin/src/wayland/quirks.h wave=wave3 loc=27
export namespace pbsd::port::wave3::kde::kwin::src::wayland::quirks {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave3::kde::kwin::src::wayland::quirks
