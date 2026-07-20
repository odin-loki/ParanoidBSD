export module pbsd.port.wave6.hbsd.src.sys.netinet6.ip6_output;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/sys/netinet6/ip6_output.c
// void ip6_output_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sys/netinet6/ip6_output.c wave=wave6 loc=3344
export namespace pbsd::port::wave6::hbsd::src::sys::netinet6::ip6_output {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave6::hbsd::src::sys::netinet6::ip6_output
