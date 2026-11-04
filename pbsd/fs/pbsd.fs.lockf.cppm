module;
#include <cstdint>

export module pbsd.fs.lockf;

import pbsd.core;

/// PROVENANCE: hbsd/src/sys/sys/fcntl.h — F_SETLK / lock types.
export namespace pbsd::fs::lockf {

enum class Type : unsigned short {
    Unlck = 0,
    Rdlck = 1,
    Wrlck = 2,
    WrlckWait = 3,
};

enum class Whence : unsigned short {
    Set = 0,
    Cur = 1,
    End = 2,
};

struct Flock {
    Type           type{};
    Whence         whence{};
    long long      start{};
    long long      len{};
};

[[nodiscard]] inline Status validate_flock(Flock const& fl) noexcept {
    if (fl.len < 0) {
        return Status::Invalid;
    }
    if (fl.type == Type::Unlck && fl.len != 0) {
        return Status::Protocol;
    }
    return Status::Ok;
}

} // namespace pbsd::fs::lockf
