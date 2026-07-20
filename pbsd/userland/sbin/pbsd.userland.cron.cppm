module;
#include <cstddef>

export module pbsd.userland.cron;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/usr.sbin/cron — schedule field parse scaffold (logic-only).
export namespace pbsd::userland::usr_sbin::cron {

struct CronField {
    int min{0};
    int max{59};
    bool any{false};
};

struct CronSchedule {
    CronField minute{};
    CronField hour{};
    CronField day{};
    CronField month{};
    CronField weekday{};
    const char* command{nullptr};
};

struct Options {
    bool foreground{false};
    bool debug{false};
    const char* user{nullptr};
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
        if (hosted::cstrcmp(arg, "-f") == 0) {
            opt.foreground = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-d") == 0) {
            opt.debug = true;
            continue;
        }
        if (arg[1] == 'u' && arg[2] != '\0') {
            opt.user = arg + 2;
            continue;
        }
        if (arg[1] == 'u' && arg[2] == '\0') {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            opt.user = argv[++i];
            continue;
        }
        return result_err<Options>(Status::Invalid);
    }
    optind_out = i;
    return result_ok(opt);
}

[[nodiscard]] inline bool parse_field_star(const char* s, CronField& f) noexcept {
    if (s == nullptr) {
        return false;
    }
    if (s[0] == '*' && s[1] == '\0') {
        f.any = true;
        return true;
    }
    int n = 0;
    for (const char* p = s; *p >= '0' && *p <= '9'; ++p) {
        n = n * 10 + (*p - '0');
    }
    if (n < f.min || n > f.max) {
        return false;
    }
    return true;
}

[[nodiscard]] inline bool crontab_line_is_comment(const char* line) noexcept {
    if (line == nullptr) {
        return true;
    }
    while (*line == ' ' || *line == '\t') {
        ++line;
    }
    return *line == '#' || *line == '\0';
}

} // namespace pbsd::userland::usr_sbin::cron
