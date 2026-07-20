export module pbsd.port.wave7.hbsd.src.stand.common.modinfo;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/stand/common/modinfo.c
// void modinfo_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/stand/common/modinfo.c wave=wave7 loc=220
export namespace pbsd::port::wave7::hbsd::src::stand::common::modinfo {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::stand::common::modinfo
