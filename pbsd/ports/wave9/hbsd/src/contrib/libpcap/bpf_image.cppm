export module pbsd.port.wave9.hbsd.src.contrib.libpcap.bpf_image;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/contrib/libpcap/bpf_image.c
// void bpf_image_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/contrib/libpcap/bpf_image.c wave=wave9 loc=424
export namespace pbsd::port::wave9::hbsd::src::contrib::libpcap::bpf_image {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave9::hbsd::src::contrib::libpcap::bpf_image
