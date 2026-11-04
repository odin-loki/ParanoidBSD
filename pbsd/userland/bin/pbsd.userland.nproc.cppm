module;
#include <cstddef>

export module pbsd.userland.nproc;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/bin/nproc/nproc.c — GNU coreutils compatible.
export namespace pbsd::userland::bin::nproc {

struct Options {
    bool all{false};
    int ignore{0};
    bool show_version{false};
    bool show_help{false};
};

[[nodiscard]] inline Result<int> parse_ignore(const char* s) noexcept {
    if (s == nullptr || s[0] == '\0') {
        return result_err<int>(Status::Invalid);
    }
    long val = 0;
    for (const char* p = s; *p != '\0'; ++p) {
        if (*p < '0' || *p > '9') {
            return result_err<int>(Status::Invalid);
        }
        val = val * 10 + (*p - '0');
        if (val > 2147483647L) {
            return result_err<int>(Status::Invalid);
        }
    }
    return result_ok(static_cast<int>(val));
}

[[nodiscard]] inline Result<Options> parse_long_option(const char* arg) noexcept {
    Options opt{};
    if (arg == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    if (hosted::cstrcmp(arg, "--all") == 0) {
        opt.all = true;
        return result_ok(opt);
    }
    if (hosted::cstrcmp(arg, "--version") == 0) {
        opt.show_version = true;
        return result_ok(opt);
    }
    if (hosted::cstrcmp(arg, "--help") == 0) {
        opt.show_help = true;
        return result_ok(opt);
    }
    constexpr const char kIgnore[] = "--ignore=";
    std::size_t klen = sizeof(kIgnore) - 1;
    bool match = true;
    for (std::size_t i = 0; i < klen; ++i) {
        if (arg[i] != kIgnore[i]) {
            match = false;
            break;
        }
    }
    if (match) {
        const auto ign = parse_ignore(arg + klen);
        if (!ign.has_value()) {
            return result_err<Options>(ign.status);
        }
        opt.ignore = ign.value;
        return result_ok(opt);
    }
    return result_err<Options>(Status::Invalid);
}

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }

    int i = 1;
    while (i < argc && argv[i] != nullptr) {
        if (argv[i][0] != '-' || argv[i][1] == '\0') {
            return result_err<Options>(Status::Invalid);
        }
        const auto one = parse_long_option(argv[i]);
        if (!one.has_value()) {
            return result_err<Options>(Status::Invalid);
        }
        if (one.value.show_version || one.value.show_help) {
            return result_ok(one.value);
        }
        if (one.value.all) {
            opt.all = true;
        }
        if (one.value.ignore > 0) {
            opt.ignore = one.value.ignore;
        }
        ++i;
    }

    if (i != argc) {
        return result_err<Options>(Status::Invalid);
    }
    return result_ok(opt);
}

[[nodiscard]] inline Result<int> cpu_count(bool all_flag) noexcept {
    int cpus = all_flag ? hosted::processors_conf() : hosted::affinity_cpu_count();
    if (cpus < 0) {
        return result_err<int>(Status::Protocol);
    }
    return result_ok(cpus);
}

[[nodiscard]] inline int apply_ignore(int cpus, int ignore) noexcept {
    if (ignore >= cpus) {
        return 1;
    }
    return cpus - ignore;
}

[[nodiscard]] inline StatusOnly run(int argc, char* const* argv, int stdout_fd = 1) noexcept {
    const auto parsed = parse_args(argc, argv);
    if (!parsed.has_value()) {
        return status_err(parsed.status);
    }
    const Options opt = parsed.value;

    if (opt.show_help || opt.show_version) {
        const char* msg = opt.show_version
            ? "nproc (neither_GNU nor_coreutils) 8.32\n"
            : "usage: nproc [--all] [--ignore=count]\n"
              "       nproc --help\n"
              "       nproc --version\n";
        return hosted::write_all(stdout_fd, msg, hosted::cstrlen(msg));
    }

    const auto raw = cpu_count(opt.all);
    if (!raw.has_value()) {
        return status_err(raw.status);
    }
    const int cpus = apply_ignore(raw.value, opt.ignore);

    char buf[32];
    unsigned pos = 0;
    int n = cpus;
    char tmp[32];
    unsigned tpos = 0;
    if (n == 0) {
        tmp[tpos++] = '0';
    } else {
        while (n > 0) {
            tmp[tpos++] = static_cast<char>('0' + (n % 10));
            n /= 10;
        }
    }
    while (tpos > 0) {
        buf[pos++] = tmp[--tpos];
    }
    buf[pos++] = '\n';
    return hosted::write_all(stdout_fd, buf, pos);
}

} // namespace pbsd::userland::bin::nproc
