export module pbsd.port.wave2.hbsd.src.sbin.kldload.kldload;

module;
extern "C" {
// Legacy TU entry — hand-port algorithm from hbsd/src/sbin/kldload/kldload.c
int main(int argc, char* argv[]);
}

export import pbsd.userland.capsicum.helpers;
import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/sbin/kldload/kldload.c wave=wave2 loc=202
export namespace pbsd::port::wave2::hbsd::src::sbin::kldload::kldload {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::sbin::kldload::kldload
