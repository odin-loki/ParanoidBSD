export module pbsd.port.wave2.hbsd.src.usr_sbin.bhyveload.bhyveload;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.sbin/bhyveload/bhyveload.c
// void bhyveload_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.sbin/bhyveload/bhyveload.c wave=wave2 loc=940
export namespace pbsd::port::wave2::hbsd::src::usr_sbin::bhyveload::bhyveload {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_sbin::bhyveload::bhyveload
