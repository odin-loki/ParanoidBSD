export module pbsd.port.wave7.hbsd.src.stand.libsa.zfs.nvlist;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/stand/libsa/zfs/nvlist.c
// void nvlist_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/stand/libsa/zfs/nvlist.c wave=wave7 loc=1698
export namespace pbsd::port::wave7::hbsd::src::stand::libsa::zfs::nvlist {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::stand::libsa::zfs::nvlist
