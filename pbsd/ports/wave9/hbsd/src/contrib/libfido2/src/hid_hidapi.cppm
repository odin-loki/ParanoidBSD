export module pbsd.port.wave9.hbsd.src.contrib.libfido2.src.hid_hidapi;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/libfido2/src/hid_hidapi.c
// void hid_hidapi_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/libfido2/src/hid_hidapi.c wave=wave9 loc=269
export namespace pbsd::port::wave9::hbsd::src::contrib::libfido2::src::hid_hidapi {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::libfido2::src::hid_hidapi
