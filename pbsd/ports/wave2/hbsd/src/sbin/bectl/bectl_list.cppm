export module pbsd.port.wave2.hbsd.src.sbin.bectl.bectl_list;

module;
extern "C" {
// Legacy TU entry — hand-port algorithm from hbsd/src/sbin/bectl/bectl_list.c
int main(int argc, char* argv[]);
}

export import pbsd.userland.capsicum.helpers;
import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sbin/bectl/bectl_list.c wave=wave2 loc=479
export namespace pbsd::port::wave2::hbsd::src::sbin::bectl::bectl_list {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::sbin::bectl::bectl_list
