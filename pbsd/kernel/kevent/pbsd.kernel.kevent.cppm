module;
#include <cstdint>

export module pbsd.kernel.kevent;

export import pbsd.core;

/// Wave 4 — kevent filters/actions/notes (sys/event.h) for kqueue/poll bridge.
export namespace pbsd::kernel::kevent {

inline constexpr int kEvFiltRead      = -1;
inline constexpr int kEvFiltWrite     = -2;
inline constexpr int kEvFiltAio       = -3;
inline constexpr int kEvFiltVnode     = -4;
inline constexpr int kEvFiltProc      = -5;
inline constexpr int kEvFiltSignal    = -6;
inline constexpr int kEvFiltTimer     = -7;
inline constexpr int kEvFiltProcdesc  = -8;
inline constexpr int kEvFiltFs        = -9;
inline constexpr int kEvFiltLio       = -10;
inline constexpr int kEvFiltUser      = -11;
inline constexpr int kEvFiltSendfile  = -12;
inline constexpr int kEvFiltEmpty     = -13;
inline constexpr int kEvFiltJail      = -14;
inline constexpr int kEvFiltJaildesc  = -15;
inline constexpr int kEvFiltSyscount  = 15;

inline constexpr unsigned short kEvAdd         = 0x0001;
inline constexpr unsigned short kEvDelete      = 0x0002;
inline constexpr unsigned short kEvEnable      = 0x0004;
inline constexpr unsigned short kEvDisable     = 0x0008;
inline constexpr unsigned short kEvOneshot     = 0x0010;
inline constexpr unsigned short kEvClear       = 0x0020;
inline constexpr unsigned short kEvReceipt     = 0x0040;
inline constexpr unsigned short kEvDispatch    = 0x0080;
inline constexpr unsigned short kEvForceoneshot = 0x0100;
inline constexpr unsigned short kEvKeepudata   = 0x0200;
inline constexpr unsigned short kEvEof         = 0x8000;
inline constexpr unsigned short kEvError       = 0x4000;

inline constexpr unsigned kNoteDelete      = 0x0001;
inline constexpr unsigned kNoteWrite       = 0x0002;
inline constexpr unsigned kNoteExtend      = 0x0004;
inline constexpr unsigned kNoteAttrib      = 0x0008;
inline constexpr unsigned kNoteLink        = 0x0010;
inline constexpr unsigned kNoteRename      = 0x0020;
inline constexpr unsigned kNoteRevoke      = 0x0040;
inline constexpr unsigned kNoteOpen        = 0x0080;
inline constexpr unsigned kNoteClose       = 0x0100;
inline constexpr unsigned kNoteCloseWrite  = 0x0200;
inline constexpr unsigned kNoteLowat       = 0x0001;
inline constexpr unsigned kNoteFilePoll    = 0x0002;

struct FilterEntry {
    int         filter;
    const char* name;
};

inline constexpr FilterEntry kFilterTable[] = {
    {kEvFiltRead,     "EVFILT_READ"},
    {kEvFiltWrite,    "EVFILT_WRITE"},
    {kEvFiltAio,      "EVFILT_AIO"},
    {kEvFiltVnode,    "EVFILT_VNODE"},
    {kEvFiltProc,     "EVFILT_PROC"},
    {kEvFiltSignal,   "EVFILT_SIGNAL"},
    {kEvFiltTimer,    "EVFILT_TIMER"},
    {kEvFiltProcdesc, "EVFILT_PROCDESC"},
    {kEvFiltFs,       "EVFILT_FS"},
    {kEvFiltLio,      "EVFILT_LIO"},
    {kEvFiltUser,     "EVFILT_USER"},
    {kEvFiltSendfile, "EVFILT_SENDFILE"},
    {kEvFiltEmpty,    "EVFILT_EMPTY"},
    {kEvFiltJail,     "EVFILT_JAIL"},
    {kEvFiltJaildesc, "EVFILT_JAILDESC"},
};

struct KeventStub {
    std::uintptr_t ident{};
    short          filter{};
    unsigned short flags{};
    unsigned       fflags{};
    std::int64_t   data{};
    void*          udata{};
};

[[nodiscard]] inline unsigned filter_table_size() noexcept {
    return static_cast<unsigned>(sizeof(kFilterTable) / sizeof(kFilterTable[0]));
}

[[nodiscard]] constexpr bool is_valid_filter(int f) noexcept {
    return f <= -1 && f >= -kEvFiltSyscount;
}

[[nodiscard]] constexpr bool is_io_filter(int f) noexcept {
    return f == kEvFiltRead || f == kEvFiltWrite || f == kEvFiltEmpty;
}

[[nodiscard]] constexpr bool is_vnode_filter(int f) noexcept {
    return f == kEvFiltVnode;
}

[[nodiscard]] constexpr bool action_is_add(unsigned short flags) noexcept {
    return (flags & kEvAdd) != 0;
}

[[nodiscard]] constexpr bool action_is_delete(unsigned short flags) noexcept {
    return (flags & kEvDelete) != 0;
}

[[nodiscard]] constexpr unsigned short merge_action(unsigned short base,
                                                    unsigned short op) noexcept {
    return static_cast<unsigned short>(base | op);
}

[[nodiscard]] inline Result<const char*> filter_name(int f) noexcept {
    for (const auto& e : kFilterTable) {
        if (e.filter == f) {
            return result_ok(e.name);
        }
    }
    return result_err<const char*>(Status::NotFound);
}

[[nodiscard]] constexpr bool validate_kevent(const KeventStub& ev) noexcept {
    return is_valid_filter(ev.filter);
}

} // namespace pbsd::kernel::kevent
