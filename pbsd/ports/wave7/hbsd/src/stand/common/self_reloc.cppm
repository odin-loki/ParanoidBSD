export module pbsd.port.wave7.hbsd.src.stand.common.self_reloc;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/stand/common/self_reloc.c
// void self_reloc_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/stand/common/self_reloc.c wave=wave7 loc=124
export namespace pbsd::port::wave7::hbsd::src::stand::common::self_reloc {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::stand::common::self_reloc
