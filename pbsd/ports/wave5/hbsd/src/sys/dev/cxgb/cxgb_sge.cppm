export module pbsd.port.wave5.hbsd.src.sys.dev.cxgb.cxgb_sge;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/dev/cxgb/cxgb_sge.c
// void cxgb_sge_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/dev/cxgb/cxgb_sge.c wave=wave5 loc=3717
export namespace pbsd::port::wave5::hbsd::src::sys::dev::cxgb::cxgb_sge {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave5::hbsd::src::sys::dev::cxgb::cxgb_sge
