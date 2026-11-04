module;
#include <cstddef>

export module pbsd.userland.sysctl;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Wave 2 sbin — sysctl(8) name parse helpers (MIB string form).
export namespace pbsd::userland::sbin::sysctl {

struct Options {
    bool write{false};     // name=value
    bool numeric{false};   // -n
    bool quiet{false};     // -q
    bool opaque{false};    // -o
    bool show_type{false}; // -t
};

/// Split "a.b.c=value" into name and optional value pointer into same buffer.
struct Assignment {
    char* name{nullptr};
    char* value{nullptr}; // nullptr if read-only query
};

[[nodiscard]] inline Assignment split_assignment(char* spec) noexcept {
    Assignment a{};
    if (spec == nullptr) {
        return a;
    }
    a.name = spec;
    for (char* p = spec; *p; ++p) {
        if (*p == '=') {
            *p = '\0';
            a.value = p + 1;
            break;
        }
    }
    return a;
}

[[nodiscard]] inline Result<Options> parse_options(int argc, char* const* argv,
                                                   int& optind_out) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-' && argv[i][1] != '\0'; ++i) {
        for (const char* p = argv[i] + 1; *p; ++p) {
            switch (*p) {
            case 'n':
                opt.numeric = true;
                break;
            case 'q':
                opt.quiet = true;
                break;
            case 'o':
                opt.opaque = true;
                break;
            case 't':
                opt.show_type = true;
                break;
            case 'w':
                opt.write = true;
                break;
            default:
                return result_err<Options>(Status::Invalid);
            }
        }
    }
    optind_out = i;
    return result_ok(opt);
}

/// Count MIB components separated by '.'.
[[nodiscard]] inline std::size_t mib_depth(const char* name) noexcept {
    if (name == nullptr || name[0] == '\0') {
        return 0;
    }
    std::size_t d = 1;
    for (const char* p = name; *p; ++p) {
        if (*p == '.') {
            ++d;
        }
    }
    return d;
}

} // namespace pbsd::userland::sbin::sysctl
