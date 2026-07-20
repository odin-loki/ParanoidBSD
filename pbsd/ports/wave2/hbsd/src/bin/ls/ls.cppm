export module pbsd.port.wave2.hbsd.src.bin.ls.ls;

module;
extern "C" {
// Legacy TU entry — hand-port algorithm from hbsd/src/bin/ls/ls.c
int main(int argc, char* argv[]);
}

export import pbsd.userland.capsicum.helpers;
import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/bin/ls/ls.c wave=wave2 loc=1071
export namespace pbsd::port::wave2::hbsd::src::bin::ls::ls {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::bin::ls::ls
