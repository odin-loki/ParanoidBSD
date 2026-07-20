export module pbsd.port.wave6.hbsd.src.sys.contrib.openzfs.lib.libnvpair.libnvpair;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/openzfs/lib/libnvpair/libnvpair.c
// void libnvpair_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/openzfs/lib/libnvpair/libnvpair.c wave=wave6 loc=1166
export namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::lib::libnvpair::libnvpair {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::contrib::openzfs::lib::libnvpair::libnvpair
