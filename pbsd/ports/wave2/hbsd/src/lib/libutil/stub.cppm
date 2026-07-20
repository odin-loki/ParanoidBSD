export module pbsd.port.wave2.hbsd.src.lib.libutil.stub;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libutil/stub.c
// void stub_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libutil/stub.c wave=wave2 loc=46
export namespace pbsd::port::wave2::hbsd::src::lib::libutil::stub {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libutil::stub
