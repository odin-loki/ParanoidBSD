export module pbsd.port.wave2.hbsd.src.lib.libnetbsd.sys.wait;

module;
// Header bridge — replace #include of hbsd/src/lib/libnetbsd/sys/wait.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libnetbsd/sys/wait.h wave=wave2 loc=36
export namespace pbsd::port::wave2::hbsd::src::lib::libnetbsd::sys::wait {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libnetbsd::sys::wait
