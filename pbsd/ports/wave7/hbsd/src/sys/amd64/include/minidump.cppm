export module pbsd.port.wave7.hbsd.src.sys.amd64.include.minidump;

module;
// Header bridge — replace #include of hbsd/src/sys/amd64/include/minidump.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/amd64/include/minidump.h wave=wave7 loc=47
export namespace pbsd::port::wave7::hbsd::src::sys::amd64::include::minidump {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::sys::amd64::include::minidump
