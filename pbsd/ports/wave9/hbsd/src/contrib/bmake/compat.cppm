export module pbsd.port.wave9.hbsd.src.contrib.bmake.compat;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/bmake/compat.c
// void compat_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/bmake/compat.c wave=wave9 loc=784
export namespace pbsd::port::wave9::hbsd::src::contrib::bmake::compat {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::bmake::compat
