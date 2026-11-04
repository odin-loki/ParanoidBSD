module;
#include <cstddef>

export module pbsd.userland.nohup;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/nohup/nohup.c — exit codes and output file (logic-only).
export namespace pbsd::userland::usr_bin::nohup {

inline constexpr const char* kOutputFilename = "nohup.out";

inline constexpr int kExitNoExec = 126;
inline constexpr int kExitNotFound = 127;
inline constexpr int kExitMisc = 127;

[[nodiscard]] inline int exec_exit_status(int errno_value) noexcept {
    return (errno_value == 2) ? kExitNotFound : kExitNoExec; // ENOENT == 2
}

[[nodiscard]] inline Result<int> parse_args(int argc, char* const* argv,
                                            int& optind_out) noexcept {
    if (argv == nullptr) {
        return result_err<int>(Status::Invalid);
    }
    int i = 1;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-'; ++i) {
        if (argv[i][1] != '\0') {
            return result_err<int>(Status::Invalid);
        }
    }
    optind_out = i;
    if (argc - i < 1) {
        return result_err<int>(Status::Invalid);
    }
    return result_ok(i);
}

[[nodiscard]] inline bool needs_output_redirect(bool stdout_tty, bool stderr_tty) noexcept {
    return stdout_tty || stderr_tty;
}

} // namespace pbsd::userland::usr_bin::nohup
