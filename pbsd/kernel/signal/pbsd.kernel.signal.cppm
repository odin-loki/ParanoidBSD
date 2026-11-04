module;
#include <cstddef>

export module pbsd.kernel.signal;

export import pbsd.core;

/// Wave 4 — signal name/number tables (subset of sys/signal.h) for kill(1)/kernel.
export namespace pbsd::kernel::signal {

inline constexpr int kSigHup = 1;
inline constexpr int kSigInt = 2;
inline constexpr int kSigQuit = 3;
inline constexpr int kSigIll = 4;
inline constexpr int kSigTrap = 5;
inline constexpr int kSigAbrt = 6;
inline constexpr int kSigEmt = 7;
inline constexpr int kSigFpe = 8;
inline constexpr int kSigKill = 9;
inline constexpr int kSigBus = 10;
inline constexpr int kSigSegv = 11;
inline constexpr int kSigSys = 12;
inline constexpr int kSigPipe = 13;
inline constexpr int kSigAlrm = 14;
inline constexpr int kSigTerm = 15;
inline constexpr int kSigUrg = 16;
inline constexpr int kSigStop = 17;
inline constexpr int kSigTstp = 18;
inline constexpr int kSigCont = 19;
inline constexpr int kSigChld = 20;
inline constexpr int kSigTtin = 21;
inline constexpr int kSigTtou = 22;
inline constexpr int kSigIo = 23;
inline constexpr int kSigXcpu = 24;
inline constexpr int kSigXfsz = 25;
inline constexpr int kSigVtalrm = 26;
inline constexpr int kSigProf = 27;
inline constexpr int kSigWinch = 28;
inline constexpr int kSigInfo = 29;
inline constexpr int kSigUsr1 = 30;
inline constexpr int kSigUsr2 = 31;

struct SigName {
    int number;
    const char* name; // without SIG prefix, uppercase
};

inline constexpr SigName kSigTable[] = {
    {kSigHup, "HUP"},   {kSigInt, "INT"},   {kSigQuit, "QUIT"},
    {kSigIll, "ILL"},   {kSigTrap, "TRAP"}, {kSigAbrt, "ABRT"},
    {kSigEmt, "EMT"},   {kSigFpe, "FPE"},   {kSigKill, "KILL"},
    {kSigBus, "BUS"},   {kSigSegv, "SEGV"}, {kSigSys, "SYS"},
    {kSigPipe, "PIPE"}, {kSigAlrm, "ALRM"}, {kSigTerm, "TERM"},
    {kSigUrg, "URG"},   {kSigStop, "STOP"}, {kSigTstp, "TSTP"},
    {kSigCont, "CONT"}, {kSigChld, "CHLD"}, {kSigTtin, "TTIN"},
    {kSigTtou, "TTOU"}, {kSigIo, "IO"},     {kSigXcpu, "XCPU"},
    {kSigXfsz, "XFSZ"}, {kSigVtalrm, "VTALRM"}, {kSigProf, "PROF"},
    {kSigWinch, "WINCH"}, {kSigInfo, "INFO"}, {kSigUsr1, "USR1"},
    {kSigUsr2, "USR2"},
};

[[nodiscard]] inline bool name_eq(const char* a, const char* b) noexcept {
    if (a == nullptr || b == nullptr) {
        return false;
    }
    while (*a && *b) {
        char ca = *a;
        char cb = *b;
        if (ca >= 'a' && ca <= 'z') {
            ca = static_cast<char>(ca - 'a' + 'A');
        }
        if (cb >= 'a' && cb <= 'z') {
            cb = static_cast<char>(cb - 'a' + 'A');
        }
        if (ca != cb) {
            return false;
        }
        ++a;
        ++b;
    }
    return *a == *b;
}

/// str2sig-like: accepts "TERM", "SIGTERM", or decimal. Returns Invalid if unknown.
[[nodiscard]] inline Result<int> str2sig(const char* s) noexcept {
    if (s == nullptr || s[0] == '\0') {
        return result_err<int>(Status::Invalid);
    }
    if (s[0] >= '0' && s[0] <= '9') {
        int n = 0;
        for (const char* p = s; *p; ++p) {
            if (*p < '0' || *p > '9') {
                return result_err<int>(Status::Invalid);
            }
            n = n * 10 + (*p - '0');
        }
        return result_ok(n);
    }
    const char* name = s;
    if ((s[0] == 'S' || s[0] == 's') && (s[1] == 'I' || s[1] == 'i')
        && (s[2] == 'G' || s[2] == 'g')) {
        name = s + 3;
    }
    for (const auto& e : kSigTable) {
        if (name_eq(name, e.name)) {
            return result_ok(e.number);
        }
    }
    return result_err<int>(Status::NotFound);
}

[[nodiscard]] inline unsigned sig_table_size() noexcept {
    return static_cast<unsigned>(sizeof(kSigTable) / sizeof(kSigTable[0]));
}

[[nodiscard]] inline Result<const char*> sig2str(int n) noexcept {
    for (const auto& e : kSigTable) {
        if (e.number == n) {
            return result_ok(e.name);
        }
    }
    return result_err<const char*>(Status::NotFound);
}

} // namespace pbsd::kernel::signal
