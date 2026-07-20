export module pbsd.port.wave2.hbsd.src.libexec.getty.chat;

module;
extern "C" {
// Legacy TU symbols — hand-port from hbsd/src/libexec/getty/chat.c
// void chat_init(void);
}

import pbsd.core;

/// Auto-generated migration stub — status: stubbed
/// @pbsd-migration-meta source=hbsd/src/libexec/getty/chat.c wave=wave2 loc=485
export namespace pbsd::port::wave2::hbsd::src::libexec::getty::chat {

[[nodiscard]] inline Status migration_status() noexcept {
    return Status::NotImplemented;
}

} // namespace pbsd::port::wave2::hbsd::src::libexec::getty::chat
