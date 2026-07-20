export module pbsd.port.wave9.hbsd.src.tools.regression.capsicum.syscalls.cap_getmode;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/tools/regression/capsicum/syscalls/cap_getmode.c
// void cap_getmode_init(void);
}

export import pbsd.userland.capsicum;
export import pbsd.userland.capsicum.helpers;
import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/tools/regression/capsicum/syscalls/cap_getmode.c wave=wave9 loc=163
export namespace pbsd::port::wave9::hbsd::src::tools::regression::capsicum::syscalls::cap_getmode {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::tools::regression::capsicum::syscalls::cap_getmode
