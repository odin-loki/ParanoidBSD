export module pbsd.port.wave4.hbsd.src.sys.kern.genoffset;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/kern/genoffset.c
// void genoffset_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/kern/genoffset.c wave=wave4 loc=39
export namespace pbsd::port::wave4::hbsd::src::sys::kern::genoffset {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave4::hbsd::src::sys::kern::genoffset
