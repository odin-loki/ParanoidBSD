module;
#include <cstddef>

export module pbsd.userland.chpass;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/usr.bin/chpass/{chpass.c,chpass.h}.
export namespace pbsd::userland::usr_bin::chpass {

enum class Operation : unsigned char {
    EditEntry,
    LoadEntry,
    NewShell,
    NewPassword,
    NewExpire,
};

enum Field : int {
    Name = 7,
    BPhone = 8,
    HPhone = 9,
    Locate = 10,
    Other = 11,
    Shell = 13,
};

struct Options {
    Operation op{Operation::EditEntry};
    bool list{false};   // -l
    bool audit{false};  // -a (load)
    bool yp{false};     // -y
    const char* arg{nullptr};
};

[[nodiscard]] inline bool accepts_flag(char c) noexcept {
    const char* allowed = "a:p:s:e:d:h:loy";
    for (const char* p = allowed; *p; ++p) {
        if (*p == c) {
            return true;
        }
    }
    return false;
}

[[nodiscard]] inline Result<Options> parse_args(int argc, char* const* argv,
                                                int& optind_out) noexcept {
    Options opt{};
    if (argv == nullptr) {
        return result_err<Options>(Status::Invalid);
    }
    int i = 1;
    for (; i < argc && argv[i] != nullptr && argv[i][0] == '-'; ++i) {
        const char* arg = argv[i];
        if (hosted::cstrcmp(arg, "--") == 0) {
            ++i;
            break;
        }
        for (int j = 1; arg[j] != '\0'; ++j) {
            switch (arg[j]) {
            case 'a':
                opt.op = Operation::LoadEntry;
                opt.audit = true;
                while (arg[j + 1] != '\0') {
                    ++j;
                }
                break;
            case 's':
                opt.op = Operation::NewShell;
                while (arg[j + 1] != '\0') {
                    ++j;
                }
                break;
            case 'p':
                opt.op = Operation::NewPassword;
                while (arg[j + 1] != '\0') {
                    ++j;
                }
                break;
            case 'e':
                opt.op = Operation::NewExpire;
                while (arg[j + 1] != '\0') {
                    ++j;
                }
                break;
            case 'l':
                opt.list = true;
                break;
            case 'o':
                opt.yp = true;
                break;
            case 'y':
                opt.yp = true;
                break;
            case 'd':
            case 'h':
                while (arg[j + 1] != '\0') {
                    ++j;
                }
                break;
            default:
                break;
            }
        }
    }
    optind_out = i;
    return result_ok(opt);
}

} // namespace pbsd::userland::usr_bin::chpass
