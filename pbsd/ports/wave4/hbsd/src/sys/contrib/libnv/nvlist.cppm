export module pbsd.port.wave4.hbsd.src.sys.contrib.libnv.nvlist;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/libnv/nvlist.c
// void nvlist_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/libnv/nvlist.c wave=wave4 loc=2064
export namespace pbsd::port::wave4::hbsd::src::sys::contrib::libnv::nvlist {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::contrib::libnv::nvlist
