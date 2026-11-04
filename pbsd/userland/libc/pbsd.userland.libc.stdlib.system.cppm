module;

export module pbsd.userland.libc.stdlib.system;

import pbsd.core;

/// system() concept from hbsd/src/lib/libc/stdlib/system.c (hosted stub)
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status system_command(const char* cmd) noexcept {
    if (cmd == nullptr) {
        return Status::NotImplemented;
    }
    return Status::NotImplemented;
}

} // namespace pbsd::userland::libc
