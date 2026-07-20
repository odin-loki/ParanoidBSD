export module pbsd.port.wave9.hbsd.src.share.examples.libusb20.bulk;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/share/examples/libusb20/bulk.c
// void bulk_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/share/examples/libusb20/bulk.c wave=wave9 loc=244
export namespace pbsd::port::wave9::hbsd::src::share::examples::libusb20::bulk {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::share::examples::libusb20::bulk
