export module pbsd.userland.hostname;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/hostname/hostname.c
export namespace pbsd::userland::bin::hostname {

enum class DisplayMode : unsigned char {
    Full,
    Short,   // -s: strip domain
    Domain,  // -d: domain portion only
};

struct Options {
    bool ignore_f{false}; // -f accepted for Linux compat
    DisplayMode mode{DisplayMode::Full};
};

struct ParsedArgs {
    Options opts{};
    const char* set_name{nullptr}; // non-null => set hostname
};

[[nodiscard]] inline Result<ParsedArgs> parse_args(int argc, char* const* argv) noexcept {
    ParsedArgs out{};
    if (argv == nullptr) {
        return result_err<ParsedArgs>(Status::Invalid);
    }

    int i = 1;
    while (i < argc && argv[i] != nullptr && argv[i][0] == '-') {
        const char* flag = argv[i];
        if (flag[1] == '\0' || flag[2] != '\0') {
            return result_err<ParsedArgs>(Status::Invalid);
        }
        switch (flag[1]) {
        case 'f':
            out.opts.ignore_f = true;
            break;
        case 's':
            if (out.opts.mode == DisplayMode::Domain) {
                return result_err<ParsedArgs>(Status::Invalid);
            }
            out.opts.mode = DisplayMode::Short;
            break;
        case 'd':
            if (out.opts.mode == DisplayMode::Short) {
                return result_err<ParsedArgs>(Status::Invalid);
            }
            out.opts.mode = DisplayMode::Domain;
            break;
        default:
            return result_err<ParsedArgs>(Status::Invalid);
        }
        ++i;
    }

    const int remaining = argc - i;
    if (remaining > 1) {
        return result_err<ParsedArgs>(Status::Invalid);
    }
    if (remaining == 1) {
        out.set_name = argv[i];
    }
    return result_ok(out);
}

/// Apply -s / -d display transforms to hostname buffer (mutates buf).
[[nodiscard]] inline const char* display_name(char* buf, DisplayMode mode) noexcept {
    if (buf == nullptr) {
        return "";
    }
    char* hostp = buf;
    char* dot = hosted::cstrchr(buf, '.');
    if (mode == DisplayMode::Short && dot != nullptr) {
        *dot = '\0';
    } else if (mode == DisplayMode::Domain && dot != nullptr) {
        hostp = dot + 1;
    }
    return hostp;
}

struct HostnameOutput {
    char buf[hosted::kMaxHostnameLen]{};
    const char* display{nullptr};
};

[[nodiscard]] inline Result<HostnameOutput> get_display(DisplayMode mode) noexcept {
    HostnameOutput out{};
    const auto st = hosted::get_hostname(out.buf, sizeof(out.buf));
    if (!ok(st.status)) {
        return result_err<HostnameOutput>(st.status);
    }
    out.display = display_name(out.buf, mode);
    return result_ok(out);
}

[[nodiscard]] inline StatusOnly run(int argc, char* const* argv, int stdout_fd = 1) noexcept {
    const auto parsed = parse_args(argc, argv);
    if (!parsed.has_value()) {
        return status_err(parsed.status);
    }
    const ParsedArgs args = parsed.value;

    if (args.set_name != nullptr) {
        return hosted::set_hostname(args.set_name);
    }

    const auto got = get_display(args.opts.mode);
    if (!got.has_value()) {
        return status_err(got.status);
    }
    return hosted::write_line(stdout_fd, got.value.display);
}

} // namespace pbsd::userland::bin::hostname
