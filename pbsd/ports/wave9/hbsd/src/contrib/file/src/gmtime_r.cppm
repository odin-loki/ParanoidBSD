export module pbsd.port.wave9.hbsd.src.contrib.file.src.gmtime_r;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/file/src/gmtime_r.c
// void gmtime_r_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/file/src/gmtime_r.c wave=wave9 loc=19
export namespace pbsd::port::wave9::hbsd::src::contrib::file::src::gmtime_r {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::file::src::gmtime_r
