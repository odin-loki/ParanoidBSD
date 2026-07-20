export module pbsd.port.wave2.hbsd.src.usr_bin.sort.radixsort;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/usr.bin/sort/radixsort.c
// void radixsort_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/usr.bin/sort/radixsort.c wave=wave2 loc=722
export namespace pbsd::port::wave2::hbsd::src::usr_bin::sort::radixsort {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::usr_bin::sort::radixsort
