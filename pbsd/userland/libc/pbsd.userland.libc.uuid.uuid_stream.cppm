module;
#include <cstddef>
#include <cstdint>

export module pbsd.userland.libc.uuid.uuid_stream;

export import pbsd.core;

/// uuid_stream from hbsd/src/lib/libc/uuid/uuid_stream.c
export namespace pbsd::userland::libc {

[[nodiscard]] inline Status uuid_stream_validate(const char* s) noexcept {
    if (s == nullptr) {
        return Status::Invalid;
    }
    return Status::Ok;
}

} // namespace pbsd::userland::libc
