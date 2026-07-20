export module pbsd.port.wave7.hbsd.src.stand.libsa.zalloc_malloc;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/stand/libsa/zalloc_malloc.c
// void zalloc_malloc_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/stand/libsa/zalloc_malloc.c wave=wave7 loc=229
export namespace pbsd::port::wave7::hbsd::src::stand::libsa::zalloc_malloc {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave7::hbsd::src::stand::libsa::zalloc_malloc
