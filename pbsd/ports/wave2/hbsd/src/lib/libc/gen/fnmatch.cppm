export module pbsd.port.wave2.hbsd.src.lib.libc.gen.fnmatch;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libc/gen/fnmatch.c
// void fnmatch_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libc/gen/fnmatch.c wave=wave2 loc=491
export namespace pbsd::port::wave2::hbsd::src::lib::libc::gen::fnmatch {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libc::gen::fnmatch
