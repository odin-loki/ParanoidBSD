module;
#include <cstddef>

export module pbsd.userland.patch;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/usr.bin/patch/patch.c — hunk/header parse scaffold.
export namespace pbsd::userland::usr_bin::patch {

struct Options {
    bool reverse{false};
    bool force{false};
    bool dry_run{false};
    bool batch{false};
    bool quiet{false};
    const char* prefix{nullptr};
    const char* suffix{nullptr};
    int strip{0};
};

struct HunkHeader {
    int old_start{0};
    int old_count{0};
    int new_start{0};
    int new_count{0};
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
        if (arg[1] == 'p' && arg[2] != '\0') {
            int n = 0;
            for (const char* p = arg + 2; *p >= '0' && *p <= '9'; ++p) {
                n = n * 10 + (*p - '0');
            }
            opt.strip = n;
            continue;
        }
        if (arg[1] == 'p' && arg[2] == '\0') {
            if (i + 1 >= argc) {
                return result_err<Options>(Status::Invalid);
            }
            int n = 0;
            for (const char* p = argv[++i]; *p >= '0' && *p <= '9'; ++p) {
                n = n * 10 + (*p - '0');
            }
            opt.strip = n;
            continue;
        }
        for (int j = 1; arg[j] != '\0'; ++j) {
            switch (arg[j]) {
            case 'R':
                opt.reverse = true;
                break;
            case 'f':
                opt.force = true;
                break;
            case 'n':
                opt.dry_run = true;
                break;
            case 't':
                opt.batch = true;
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
    }
    optind_out = i;
    return result_ok(opt);
}

[[nodiscard]] inline bool parse_hunk_header(const char* line, HunkHeader& hdr) noexcept {
    if (line == nullptr || line[0] != '@' || line[1] != '@') {
        return false;
    }
    int os = 0, oc = 0, ns = 0, nc = 0;
    const char* p = line + 2;
    while (*p == ' ') {
        ++p;
    }
    if (*p != '-') {
        return false;
    }
    ++p;
    while (*p >= '0' && *p <= '9') {
        os = os * 10 + (*p - '0');
        ++p;
    }
    if (*p == ',') {
        ++p;
        while (*p >= '0' && *p <= '9') {
            oc = oc * 10 + (*p - '0');
            ++p;
        }
    } else {
        oc = 1;
    }
    while (*p == ' ') {
        ++p;
    }
    if (*p != '+') {
        return false;
    }
    ++p;
    while (*p >= '0' && *p <= '9') {
        ns = ns * 10 + (*p - '0');
        ++p;
    }
    if (*p == ',') {
        ++p;
        while (*p >= '0' && *p <= '9') {
            nc = nc * 10 + (*p - '0');
            ++p;
        }
    } else {
        nc = 1;
    }
    hdr.old_start = os;
    hdr.old_count = oc;
    hdr.new_start = ns;
    hdr.new_count = nc;
    return true;
}

} // namespace pbsd::userland::usr_bin::patch
