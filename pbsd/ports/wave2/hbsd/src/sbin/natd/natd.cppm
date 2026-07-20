export module pbsd.port.wave2.hbsd.src.sbin.natd.natd;

module;
extern "C" {
// Legacy TU entry — hand-port algorithm from hbsd/src/sbin/natd/natd.c
int main(int argc, char* argv[]);
}

export import pbsd.userland.capsicum.helpers;
import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sbin/natd/natd.c wave=wave2 loc=2048
export namespace pbsd::port::wave2::hbsd::src::sbin::natd::natd {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::sbin::natd::natd
