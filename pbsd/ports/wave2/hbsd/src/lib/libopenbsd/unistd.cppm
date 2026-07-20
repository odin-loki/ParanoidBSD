export module pbsd.port.wave2.hbsd.src.lib.libopenbsd.unistd;

module;
// Header bridge — replace #include of hbsd/src/lib/libopenbsd/unistd.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libopenbsd/unistd.h wave=wave2 loc=34
export namespace pbsd::port::wave2::hbsd::src::lib::libopenbsd::unistd {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libopenbsd::unistd
