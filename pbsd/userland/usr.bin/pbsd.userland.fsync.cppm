module;
#include <cstddef>

export module pbsd.userland.fsync;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/fsync/fsync.c — argc guard (logic-only).
export namespace pbsd::userland::usr_bin::fsync {

inline constexpr int kExitOk = 0;
inline constexpr int kExitNoInput = 66;

struct Options {};

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv,
                                                int& optind_out) noexcept {
    Options opt{};
    (void)opt;
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    optind_out = 1;
    if (argc < 2) {
        return result_err<Options>(Status::Invalid);
    }
    return result_ok(opt);
}

[[nodiscard]] inline int file_count(int argc) noexcept {
    return argc >= 2 ? argc - 1 : 0;
}

[[nodiscard]] inline bool needs_at_least_one_path(int argc) noexcept {
    return argc >= 2;
}

} // namespace pbsd::userland::usr_bin::fsync
