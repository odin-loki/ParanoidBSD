export module pbsd.port.wave7.hbsd.src.sys.arm64.include.signal;

module;
// Header bridge — replace #include of hbsd/src/sys/arm64/include/signal.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/arm64/include/signal.h wave=wave7 loc=53
export namespace pbsd::port::wave7::hbsd::src::sys::arm64::include::signal {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::sys::arm64::include::signal
