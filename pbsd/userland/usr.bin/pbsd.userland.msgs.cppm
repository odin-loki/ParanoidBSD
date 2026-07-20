module;
#include <cstddef>

export module pbsd.userland.msgs;

export import pbsd.core;

/// Port of hbsd/src/usr.bin/msgs/msgs.c — bulletin-board option parsing (logic-only).
export namespace pbsd::userland::usr_bin::msgs {

inline constexpr int kModeRead = 0;
inline constexpr int kModeSend = 1;
inline constexpr int kModeClean = 2;

[[nodiscard]] inline Result<int> parse_mode_flag(char ch) noexcept {
    switch (ch) {
    case 's':
        return result_ok(kModeSend);
    case 'c':
        return result_ok(kModeClean);
    case 'f':
    case 'h':
    case 'l':
    case 'o':
    case 'p':
    case 'q':
        return result_ok(kModeRead);
    default:
        return result_err<int>(Status::Invalid);
    }
}

[[nodiscard]] inline bool is_prompt_command(char ch) noexcept {
    switch (ch) {
    case 'y':
    case 'n':
    case 'q':
    case 'p':
    case 'P':
    case '-':
    case 's':
    case 'm':
    case 'x':
        return true;
    default:
        return ch >= '0' && ch <= '9';
    }
}

[[nodiscard]] inline Result<int> parse_args(int argc, char* const* argv,
                                            int& optind_out) noexcept {
    if (argv == nullptr) {
        return result_err<int>(Status::Invalid);
    }
    optind_out = 1;
    return result_ok(0);
}

} // namespace pbsd::userland::usr_bin::msgs
