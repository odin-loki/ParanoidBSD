module;
#include <cstddef>

export module pbsd.userland.libc.gen.confstr;

export import pbsd.core;

/// confstr scaffold from hbsd/src/lib/libc/gen/confstr.c
export namespace pbsd::userland::libc {

enum class ConfStr : int {
    Path = 1,
    V7Unix98 = 2,
};

[[nodiscard]] inline StatusOnly confstr(ConfStr name, char* buf, std::size_t len) noexcept {
    if (buf == nullptr && len != 0) {
        return status_err(Status::Invalid);
    }
    switch (name) {
    case ConfStr::Path:
    case ConfStr::V7Unix98:
        if (len > 0 && buf != nullptr) {
            buf[0] = '\0';
        }
        return status_err(Status::NotImplemented);
    }
    return status_err(Status::Invalid);
}

} // namespace pbsd::userland::libc
