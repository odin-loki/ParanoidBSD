export module pbsd.userland.domainname;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/domainname/domainname.c
export namespace pbsd::userland::bin::domainname {

struct ParsedArgs {
    const char* set_name{nullptr};
};

[[nodiscard]] inline Result<ParsedArgs> parse_args(int argc, char* const* argv) noexcept {
    ParsedArgs out{};
    if (argv == nullptr) {
        return result_err<ParsedArgs>(Status::Invalid);
    }

    int i = 1;
    while (i < argc && argv[i] != nullptr) {
        if (argv[i][0] == '-') {
            return result_err<ParsedArgs>(Status::Invalid);
        }
        ++i;
    }

    const int remaining = argc - 1;
    if (remaining > 1) {
        return result_err<ParsedArgs>(Status::Invalid);
    }
    if (remaining == 1) {
        out.set_name = argv[1];
    }
    return result_ok(out);
}

struct DomainOutput {
    char buf[hosted::kMaxHostnameLen]{};
};

[[nodiscard]] inline Result<DomainOutput> get_domain() noexcept {
    DomainOutput out{};
    const auto st = hosted::get_domainname(out.buf, sizeof(out.buf));
    if (!ok(st.status)) {
        return result_err<DomainOutput>(st.status);
    }
    return result_ok(out);
}

[[nodiscard]] inline StatusOnly run(int argc, char* const* argv, int stdout_fd = 1) noexcept {
    const auto parsed = parse_args(argc, argv);
    if (!parsed.has_value()) {
        return status_err(parsed.status);
    }

    if (parsed.value.set_name != nullptr) {
        return hosted::set_domainname(parsed.value.set_name);
    }

    const auto got = get_domain();
    if (!got.has_value()) {
        return status_err(got.status);
    }
    return hosted::write_line(stdout_fd, got.value.buf);
}

} // namespace pbsd::userland::bin::domainname
