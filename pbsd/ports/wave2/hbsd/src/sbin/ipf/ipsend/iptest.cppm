export module pbsd.port.wave2.hbsd.src.sbin.ipf.ipsend.iptest;

module;
extern "C" {
// Legacy TU entry — hand-port algorithm from hbsd/src/sbin/ipf/ipsend/iptest.c
int main(int argc, char* argv[]);
}

export import pbsd.userland.capsicum.helpers;
import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sbin/ipf/ipsend/iptest.c wave=wave2 loc=191
export namespace pbsd::port::wave2::hbsd::src::sbin::ipf::ipsend::iptest {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::sbin::ipf::ipsend::iptest
