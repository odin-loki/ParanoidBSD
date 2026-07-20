export module pbsd.port.wave2.hbsd.src.bin.csh.iconv_stub;

module;
extern "C" {
// Legacy TU entry — hand-port algorithm from hbsd/src/bin/csh/iconv_stub.c
int main(int argc, char* argv[]);
}

export import pbsd.userland.capsicum.helpers;
import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/bin/csh/iconv_stub.c wave=wave2 loc=78
export namespace pbsd::port::wave2::hbsd::src::bin::csh::iconv_stub {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::bin::csh::iconv_stub
