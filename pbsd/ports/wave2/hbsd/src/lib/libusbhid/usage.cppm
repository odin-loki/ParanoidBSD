export module pbsd.port.wave2.hbsd.src.lib.libusbhid.usage;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/lib/libusbhid/usage.c
// void usage_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/lib/libusbhid/usage.c wave=wave2 loc=238
export namespace pbsd::port::wave2::hbsd::src::lib::libusbhid::usage {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::lib::libusbhid::usage
