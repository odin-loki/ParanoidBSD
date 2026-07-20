module;
#include <cstddef>

export module pbsd.userland.calendar;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port of hbsd/src/usr.bin/calendar/calendar.c — flag parse stub (logic-only).
export namespace pbsd::userland::usr_bin::calendar {

struct Options {
    bool all{false};
    bool debug{false};
    bool friday{false};
    int day_before{0};
    int day_after{0};
    const char* calendar_file{nullptr};
    const char* locale{nullptr};
};

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv,
                                                int& optind_out) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    for (; i < argc && argv[i] != nullptr; ++i) {
        const char* arg = argv[i];
        if (arg[0] != '-') {
            break;
        }
        if (hosted::cstrcmp(arg, "-") == 0 || hosted::cstrcmp(arg, "-a") == 0 ||
            hosted::cstrcmp(arg, "--all") == 0) {
            opt.all = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-A") == 0) {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            ++i;
            continue;
        }
        if (hosted::cstrcmp(arg, "-B") == 0) {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            ++i;
            continue;
        }
        if (hosted::cstrcmp(arg, "-D") == 0) {
            opt.debug = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-F") == 0) {
            opt.friday = true;
            continue;
        }
        if (hosted::cstrcmp(arg, "-f") == 0) {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            opt.calendar_file = argv[++i];
            continue;
        }
        if (hosted::cstrcmp(arg, "-l") == 0) {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            opt.locale = argv[++i];
            continue;
        }
        return result_err<Options>(Status::Invalid);
    }
    optind_out = i;
    return result_ok(opt);
}

[[nodiscard]] inline bool is_month_token(const char* s) noexcept {
    if (s == nullptr || s[0] == '\0') {
        return false;
    }
    static const char* const months[] = {
        "jan", "feb", "mar", "apr", "may", "jun",
        "jul", "aug", "sep", "oct", "nov", "dec", nullptr};
    for (const char* const* p = months; *p != nullptr; ++p) {
        if (hosted::cstrcmp(s, *p) == 0) {
            return true;
        }
    }
    return false;
}

} // namespace pbsd::userland::usr_bin::calendar
