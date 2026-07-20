module;

export module pbsd.userland.libc.stdlib.exit;

/// exit status helpers from hbsd/src/lib/libc/stdlib/exit.c
export namespace pbsd::userland::libc {

enum class ExitStatus : int {
    Success = 0,
    Failure = 1,
    UsageError = 64,
    DataError = 65,
    NoInput = 66,
    NoUser = 67,
    NoHost = 68,
    Unavailable = 69,
    Software = 70,
    OsError = 71,
    OsFile = 72,
    CantCreate = 73,
    IoError = 74,
    TempFail = 75,
    Protocol = 76,
    NoPerm = 77,
    Config = 78,
};

[[nodiscard]] inline int exit_code(ExitStatus s) noexcept {
    return static_cast<int>(s);
}

} // namespace pbsd::userland::libc
