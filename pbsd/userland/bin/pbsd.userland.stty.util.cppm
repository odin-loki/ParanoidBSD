module;
#include <cstdint>

export module pbsd.userland.stty.util;

export import pbsd.core;

/// Port of hbsd/src/bin/stty/util.c — redirect/control-tty warning helper.
export namespace pbsd::userland::bin::stty::util {

struct TtyDevice {
    std::uint64_t rdev{0};
    bool is_tty{false};
};

[[nodiscard]] inline bool stdout_appears_redirected(const TtyDevice& out,
                                                    const TtyDevice& err) noexcept {
    return out.is_tty && err.is_tty && out.rdev != err.rdev;
}

[[nodiscard]] inline bool should_warn_redirect(const TtyDevice& out,
                                               const TtyDevice& err) noexcept {
    return stdout_appears_redirected(out, err);
}

} // namespace pbsd::userland::bin::stty::util
