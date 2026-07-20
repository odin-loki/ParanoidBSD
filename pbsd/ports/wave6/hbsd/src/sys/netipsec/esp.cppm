export module pbsd.port.wave6.hbsd.src.sys.netipsec.esp;

module;
// Header bridge — replace #include of hbsd/src/sys/netipsec/esp.h
// with imports from pbsd.userland.capsicum.helpers during hand-port.

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netipsec/esp.h wave=wave6 loc=68
export namespace pbsd::port::wave6::hbsd::src::sys::netipsec::esp {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netipsec::esp
