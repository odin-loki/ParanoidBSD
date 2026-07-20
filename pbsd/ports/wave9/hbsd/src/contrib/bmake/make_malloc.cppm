export module pbsd.port.wave9.hbsd.src.contrib.bmake.make_malloc;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/bmake/make_malloc.c
// void make_malloc_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/bmake/make_malloc.c wave=wave9 loc=96
export namespace pbsd::port::wave9::hbsd::src::contrib::bmake::make_malloc {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::bmake::make_malloc
