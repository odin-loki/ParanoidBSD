module;
#include <cstddef>

export module pbsd.userland.logname;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/logname/logname.c — argc guard (logic-only).
export namespace pbsd::userland::usr_bin::logname {

[[nodiscard]] inline Result<int> parse_args(int argc, char* const* argv) noexcept {
    (void)argv;
    if (argc != 1) {
        return result_err<int>(Status::Invalid);
    }
    return result_ok(0);
}

[[nodiscard]] inline bool expects_no_arguments(int argc) noexcept {
    return argc == 1;
}

} // namespace pbsd::userland::usr_bin::logname
