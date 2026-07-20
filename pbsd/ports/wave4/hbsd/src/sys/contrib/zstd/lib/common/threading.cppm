export module pbsd.port.wave4.hbsd.src.sys.contrib.zstd.lib.common.threading;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/contrib/zstd/lib/common/threading.c
// void threading_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/contrib/zstd/lib/common/threading.c wave=wave4 loc=182
export namespace pbsd::port::wave4::hbsd::src::sys::contrib::zstd::lib::common::threading {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::contrib::zstd::lib::common::threading
