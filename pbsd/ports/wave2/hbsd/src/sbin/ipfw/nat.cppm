export module pbsd.port.wave2.hbsd.src.sbin.ipfw.nat;

module;
extern "C" {
// Legacy TU entry — hand-port algorithm from hbsd/src/sbin/ipfw/nat.c
int main(int argc, char* argv[]);
}

export import pbsd.userland.capsicum.helpers;
import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sbin/ipfw/nat.c wave=wave2 loc=1199
export namespace pbsd::port::wave2::hbsd::src::sbin::ipfw::nat {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::sbin::ipfw::nat
