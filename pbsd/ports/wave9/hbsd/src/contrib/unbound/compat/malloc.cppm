export module pbsd.port.wave9.hbsd.src.contrib.unbound.compat.malloc;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/unbound/compat/malloc.c
// void malloc_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/unbound/compat/malloc.c wave=wave9 loc=20
export namespace pbsd::port::wave9::hbsd::src::contrib::unbound::compat::malloc {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::unbound::compat::malloc
