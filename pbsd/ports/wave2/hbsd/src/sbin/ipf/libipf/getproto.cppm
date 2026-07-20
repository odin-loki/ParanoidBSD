export module pbsd.port.wave2.hbsd.src.sbin.ipf.libipf.getproto;

module;
extern "C" {
// Legacy TU entry — hand-port algorithm from hbsd/src/sbin/ipf/libipf/getproto.c
int main(int argc, char* argv[]);
}

export import pbsd.userland.capsicum.helpers;
import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sbin/ipf/libipf/getproto.c wave=wave2 loc=34
export namespace pbsd::port::wave2::hbsd::src::sbin::ipf::libipf::getproto {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::sbin::ipf::libipf::getproto
