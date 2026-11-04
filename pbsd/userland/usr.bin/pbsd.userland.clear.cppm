module;
#include <cstddef>

export module pbsd.userland.clear;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/clear/clear.c — terminal clear sequence (logic-only).
export namespace pbsd::userland::usr_bin::clear {

inline constexpr const char* kClearSequence = "\033[H\033[2J";

[[nodiscard]] inline std::size_t sequence_length() noexcept {
    return hosted::cstrlen(kClearSequence);
}

[[nodiscard]] inline Result<int> parse_args(int argc, char* const* argv) noexcept {
    if (argv == nullptr) {
        return result_err<int>(Status::Invalid);
    }
    if (argc != 1) {
        return result_err<int>(Status::Invalid);
    }
    return result_ok(0);
}

} // namespace pbsd::userland::usr_bin::clear
