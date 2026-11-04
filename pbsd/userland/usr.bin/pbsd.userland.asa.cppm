module;
#include <cstddef>

export module pbsd.userland.asa;

export import pbsd.core;

/// Port of hbsd/src/usr.bin/asa/asa.c — Fortran carriage control (logic-only).
export namespace pbsd::userland::usr_bin::asa {

enum class ControlAction : unsigned char { None, Newline, FormFeed, CarriageReturn };

[[nodiscard]] inline ControlAction classify_control(char ctrl, bool had_eol) noexcept {
    switch (ctrl) {
    case '0':
        return ControlAction::Newline;
    case '1':
        return ControlAction::FormFeed;
    case '+':
        return had_eol ? ControlAction::CarriageReturn : ControlAction::None;
    case ' ':
    default:
        return ControlAction::None;
    }
}

[[nodiscard]] inline bool needs_prior_newline(char ctrl) noexcept {
    return ctrl != '+';
}

[[nodiscard]] inline std::size_t payload_offset(char ctrl, std::size_t len) noexcept {
    if (len <= 1) {
        return 0;
    }
    if (ctrl == '\0') {
        return len;
    }
    if (ctrl != ' ' && ctrl != '0' && ctrl != '1' && ctrl != '+') {
        return len;
    }
    return len - 1;
}

[[nodiscard]] inline Result<int> parse_args(int argc, char* const* argv) noexcept {
    (void)argv;
    if (argc < 1) {
        return result_err<int>(Status::Invalid);
    }
    return result_ok(0);
}

} // namespace pbsd::userland::usr_bin::asa
