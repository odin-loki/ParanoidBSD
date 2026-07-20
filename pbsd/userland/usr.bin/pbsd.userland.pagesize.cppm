module;
#include <cstddef>

export module pbsd.userland.pagesize;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/pagesize/pagesize.sh — page size query (logic-only).
export namespace pbsd::userland::usr_bin::pagesize {

inline constexpr const char* kSysctlName = "hw.pagesize";

[[nodiscard]] inline Result<int> parse_args(int argc, char* const* argv) noexcept {
    if (argv == nullptr) {
        return result_err<int>(Status::Invalid);
    }
    if (argc != 1) {
        return result_err<int>(Status::Invalid);
    }
    return result_ok(0);
}

[[nodiscard]] inline bool valid_pagesize(long value) noexcept {
    return value > 0 && (value & (value - 1)) == 0;
}

} // namespace pbsd::userland::usr_bin::pagesize
