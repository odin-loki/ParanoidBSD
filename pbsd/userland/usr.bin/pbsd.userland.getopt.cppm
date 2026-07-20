module;

#include <cstddef>

export module pbsd.userland.getopt;

export import pbsd.core;
export import pbsd.userland.hosted;
export import pbsd.userland.capsicum.helpers;

/// Port of hbsd/src/usr.bin/getopt/getopt.c — parse argv with a supplied optstring.
export namespace pbsd::userland::usr_bin::getopt {

struct ParsedOption {
    int         opt{0};
    const char* arg{nullptr};
};

struct ParseResult {
    int           status{0};
    unsigned      opt_count{0};
    ParsedOption  options[64]{};
    unsigned      arg_count{0};
    const char*   args[64]{};
};

[[nodiscard]] inline StatusOnly parse(int argc, char* const* argv,
                                      ParseResult& out) noexcept {
    if (argv == nullptr || argv[0] == nullptr || argv[1] == nullptr) {
        return status_err(Status::Invalid);
    }

    out = ParseResult{};
    const char* optstring = argv[1];
    int optind = 2;

    while (optind < argc && argv[optind] != nullptr) {
        const char* token = argv[optind];
        if (token[0] != '-' || token[1] == '\0') {
            break;
        }

        const char opt = token[1];
        bool matched = false;
        for (const char* p = optstring; *p != '\0'; ++p) {
            if (*p == opt) {
                matched = true;
                if (p[1] == ':') {
                    if (token[2] != '\0') {
                        if (out.opt_count >= 64) {
                            return status_err(Status::Invalid);
                        }
                        out.options[out.opt_count++] = ParsedOption{opt, token + 2};
                    } else if (optind + 1 < argc && argv[optind + 1] != nullptr) {
                        if (out.opt_count >= 64) {
                            return status_err(Status::Invalid);
                        }
                        out.options[out.opt_count++] = ParsedOption{opt, argv[optind + 1]};
                        ++optind;
                    } else {
                        out.status = 1;
                    }
                } else {
                    if (out.opt_count >= 64) {
                        return status_err(Status::Invalid);
                    }
                    out.options[out.opt_count++] = ParsedOption{opt, nullptr};
                }
                break;
            }
        }
        if (!matched) {
            out.status = 1;
        }
        ++optind;
    }

    for (; optind < argc && argv[optind] != nullptr; ++optind) {
        if (out.arg_count >= 64) {
            return status_err(Status::Invalid);
        }
        out.args[out.arg_count++] = argv[optind];
    }
    return status_ok();
}

[[nodiscard]] inline StatusOnly append_cstr(char* buf, std::size_t cap,
                                              std::size_t& pos,
                                              const char* s) noexcept {
    if (buf == nullptr || s == nullptr) {
        return status_err(Status::Invalid);
    }
    while (*s != '\0') {
        if (pos + 1 >= cap) {
            return status_err(Status::Invalid);
        }
        buf[pos++] = *s++;
    }
    buf[pos] = '\0';
    return status_ok();
}

[[nodiscard]] inline StatusOnly format_output(const ParseResult& parsed, char* buf,
                                              std::size_t cap) noexcept {
    if (buf == nullptr || cap == 0) {
        return status_err(Status::Invalid);
    }
    std::size_t pos = 0;
    buf[0] = '\0';

    for (unsigned i = 0; i < parsed.opt_count; ++i) {
        const auto& row = parsed.options[i];
        if (append_cstr(buf, cap, pos, " -").status != Status::Ok) {
            return status_err(Status::Invalid);
        }
        if (pos + 2 >= cap) {
            return status_err(Status::Invalid);
        }
        buf[pos++] = static_cast<char>(row.opt);
        buf[pos] = '\0';
        if (row.arg != nullptr) {
            if (append_cstr(buf, cap, pos, " ").status != Status::Ok) {
                return status_err(Status::Invalid);
            }
            if (append_cstr(buf, cap, pos, row.arg).status != Status::Ok) {
                return status_err(Status::Invalid);
            }
        }
    }

    if (append_cstr(buf, cap, pos, " --").status != Status::Ok) {
        return status_err(Status::Invalid);
    }
    for (unsigned i = 0; i < parsed.arg_count; ++i) {
        if (append_cstr(buf, cap, pos, " ").status != Status::Ok) {
            return status_err(Status::Invalid);
        }
        if (append_cstr(buf, cap, pos, parsed.args[i]).status != Status::Ok) {
            return status_err(Status::Invalid);
        }
    }
    if (append_cstr(buf, cap, pos, "\n").status != Status::Ok) {
        return status_err(Status::Invalid);
    }
    return status_ok();
}

[[nodiscard]] inline StatusOnly run(int argc, char* const* argv,
                                    int stdout_fd = 1) noexcept {
    if (capsicum::limit_stdio() != Status::Ok) {
        return status_err(Status::Denied);
    }
    if (capsicum::enter_sandbox() != Status::Ok) {
        return status_err(Status::Denied);
    }

    ParseResult parsed{};
    const auto st = parse(argc, argv, parsed);
    if (!ok(st.status)) {
        return st;
    }

    char line[4096];
    if (format_output(parsed, line, sizeof(line)).status != Status::Ok) {
        return status_err(Status::Invalid);
    }
    return hosted::write_all(stdout_fd, line, hosted::cstrlen(line));
}

[[nodiscard]] inline int exit_code(const ParseResult& parsed) noexcept {
    return parsed.status;
}

} // namespace pbsd::userland::usr_bin::getopt
