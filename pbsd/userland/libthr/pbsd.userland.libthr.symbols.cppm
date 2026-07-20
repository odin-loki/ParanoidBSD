module;

#include <cstddef>
#include <cstdint>

export module pbsd.userland.libthr.symbols;

/// Debugger symbol offsets from hbsd/src/lib/libthr/thread/thr_symbols.c
export namespace pbsd::userland::libthr::symbols {

inline constexpr int kThreadOffTcb = 0;
inline constexpr int kThreadOffTid = 8;
inline constexpr int kThreadOffNext = 16;
inline constexpr int kThreadOffAttrFlags = 24;
inline constexpr int kThreadOffLinkmap = 32;
inline constexpr int kThreadOffTlsIndex = 40;
inline constexpr int kThreadOffReportEvents = 48;
inline constexpr int kThreadOffEventMask = 52;
inline constexpr int kThreadOffEventBuf = 56;
inline constexpr int kThreadSizeKey = 16;
inline constexpr int kThreadOffKeyAllocated = 0;
inline constexpr int kThreadOffKeyDestructor = 8;
inline constexpr int kThreadMaxKeys = 256;
inline constexpr int kThreadOffDtv = 0;
inline constexpr int kThreadOffState = 64;
inline constexpr int kThreadStateRunning = 1;
inline constexpr int kThreadStateZombie = 4;

struct DebugOffsets {
    int off_tcb;
    int off_tid;
    int off_next;
    int off_state;
    int max_keys;
};

[[nodiscard]] inline DebugOffsets default_offsets() noexcept {
    return {kThreadOffTcb, kThreadOffTid, kThreadOffNext, kThreadOffState, kThreadMaxKeys};
}

} // namespace pbsd::userland::libthr::symbols
