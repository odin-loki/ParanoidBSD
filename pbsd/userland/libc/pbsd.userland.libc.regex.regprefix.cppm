module;
#include <cstddef>

export module pbsd.userland.libc.regex.regprefix;

export import pbsd.core;

/// regprefix scaffold from hbsd/src/lib/libc/regex/regprefix.c (BSD extension)
export namespace pbsd::userland::libc::regex {

[[nodiscard]] inline StatusOnly regprefix(const void* preg, const char** prefix, std::size_t* len) noexcept {
    (void)preg;
    if (prefix != nullptr) {
        *prefix = "";
    }
    if (len != nullptr) {
        *len = 0;
    }
    return status_err(Status::NotImplemented);
}

} // namespace pbsd::userland::libc::regex
