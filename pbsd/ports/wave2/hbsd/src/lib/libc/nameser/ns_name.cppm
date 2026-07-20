export module pbsd.port.wave2.hbsd.src.lib.libc.nameser.ns_name;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/nameser/ns_name.c
// void ns_name_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/nameser/ns_name.c wave=wave2 loc=1152
export namespace pbsd::port::wave2::hbsd::src::lib::libc::nameser::ns_name {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::nameser::ns_name
