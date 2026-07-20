export module pbsd.port.wave2.hbsd.src.lib.libc.include.port_before;

module;
// Header bridge — replace #include of hbsd/src/lib/libc/include/port_before.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/include/port_before.h wave=wave2 loc=21
export namespace pbsd::port::wave2::hbsd::src::lib::libc::include::port_before {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::include::port_before
