export module pbsd.port.wave2.hbsd.src.lib.libutil.kinfo_getallproc;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libutil/kinfo_getallproc.c
// void kinfo_getallproc_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libutil/kinfo_getallproc.c wave=wave2 loc=95
export namespace pbsd::port::wave2::hbsd::src::lib::libutil::kinfo_getallproc {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libutil::kinfo_getallproc
