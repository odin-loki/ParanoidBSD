export module pbsd.port.wave9.hbsd.src.tools.build.dummy;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/tools/build/dummy.c
// void dummy_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/tools/build/dummy.c wave=wave9 loc=4
export namespace pbsd::port::wave9::hbsd::src::tools::build::dummy {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::tools::build::dummy
