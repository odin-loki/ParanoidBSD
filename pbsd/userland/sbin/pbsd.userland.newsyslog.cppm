module;
#include <cstddef>

export module pbsd.userland.newsyslog;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/usr.sbin/newsyslog — conf-line parse scaffold (logic-only).
export namespace pbsd::userland::usr_sbin::newsyslog {

struct LogEntry {
    const char* logfile{nullptr};
    const char* owner{nullptr};
    const char* mode{nullptr};
    int count{7};
    int size_kb{0};
    const char* pidfile{nullptr};
    const char* signal{nullptr};
    bool compress{false};
    bool no_compress{false};
    bool no_jitter{false};
};

struct Options {
    bool dry_run{false};
    bool verbose{false};
    bool force{false};
    const char* config{nullptr};
};

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv,
                                                int& optind_out) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-'; ++i) {
        const char* arg = argv[i];
        if (arg[1] == 'C' && arg[2] != '\0') {
            opt.config = arg + 2;
            continue;
        }
        if (arg[1] == 'C' && arg[2] == '\0') {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            opt.config = argv[++i];
            continue;
        }
        for (int j = 1; arg[j] != '\0'; ++j) {
            switch (arg[j]) {
            case 'n':
                opt.dry_run = true;
                break;
            case 'v':
                opt.verbose = true;
                break;
            case 'F':
                opt.force = true;
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
    }
    optind_out = i;
    return result_ok(opt);
}

[[nodiscard]] inline bool conf_line_is_comment(const char* line) noexcept {
    if (line == nullptr) {
        return true;
    }
    while (*line == ' ' || *line == '\t') {
        ++line;
    }
    return *line == '#' || *line == '\0';
}

[[nodiscard]] inline int parse_count_field(const char* s) noexcept {
    if (s == nullptr || s[0] == '*') {
        return -1;
    }
    int n = 0;
    for (const char* p = s; *p >= '0' && *p <= '9'; ++p) {
        n = n * 10 + (*p - '0');
    }
    return n;
}

} // namespace pbsd::userland::usr_sbin::newsyslog
