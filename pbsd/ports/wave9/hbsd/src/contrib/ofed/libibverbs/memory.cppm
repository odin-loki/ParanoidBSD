export module pbsd.port.wave9.hbsd.src.contrib.ofed.libibverbs.memory;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/ofed/libibverbs/memory.c
// void memory_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/ofed/libibverbs/memory.c wave=wave9 loc=704
export namespace pbsd::port::wave9::hbsd::src::contrib::ofed::libibverbs::memory {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::ofed::libibverbs::memory
