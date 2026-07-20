export module pbsd.kernel.errno;

export import pbsd.core;

/// Wave 4 — POSIX/BSD errno subset as typed Status bridge (from sys/errno.h).
export namespace pbsd::kernel::errno_ns {

enum class Errno : int {
    Ok = 0,
    Perm = 1,       // EPERM
    NoEnt = 2,      // ENOENT
    Srch = 3,       // ESRCH
    Intr = 4,       // EINTR
    Io = 5,         // EIO
    NxIo = 6,       // ENXIO
    TooBig = 7,     // E2BIG
    NoExec = 8,     // ENOEXEC
    BadF = 9,       // EBADF
    Child = 10,     // ECHILD
    DeadLk = 11,    // EDEADLK
    NoMem = 12,     // ENOMEM
    Acces = 13,     // EACCES
    Fault = 14,     // EFAULT
    Busy = 16,      // EBUSY
    Exist = 17,     // EEXIST
    XDev = 18,      // EXDEV
    NoDev = 19,     // ENODEV
    NotDir = 20,    // ENOTDIR
    IsDir = 21,     // EISDIR
    Inval = 22,     // EINVAL
    NFile = 23,     // ENFILE
    MFile = 24,     // EMFILE
    NotTy = 25,     // ENOTTY
    TxtBsy = 26,    // ETXTBSY
    FBig = 27,      // EFBIG
    NoSpc = 28,     // ENOSPC
    SPipe = 29,     // ESPIPE
    Rofs = 30,      // EROFS
    MLink = 31,     // EMLINK
    Pipe = 32,      // EPIPE
    Dom = 33,       // EDOM
    Range = 34,     // ERANGE
    Again = 35,     // EAGAIN
    TimedOut = 60,  // ETIMEDOUT (FreeBSD)
    NotSupp = 45,   // ENOTSUP
    CapMode = 94,   // ECAPMODE (Capsicum)
    NotCapable = 93,// ENOTCAPABLE
};

[[nodiscard]] constexpr Status to_status(Errno e) noexcept {
    switch (e) {
    case Errno::Ok:
        return Status::Ok;
    case Errno::Perm:
    case Errno::Acces:
    case Errno::Rofs:
    case Errno::CapMode:
    case Errno::NotCapable:
        return Status::Denied;
    case Errno::NoEnt:
    case Errno::NoDev:
    case Errno::Srch:
        return Status::NotFound;
    case Errno::NoMem:
    case Errno::NoSpc:
        return Status::NoMemory;
    case Errno::Busy:
    case Errno::TxtBsy:
    case Errno::DeadLk:
        return Status::Busy;
    case Errno::TimedOut:
        return Status::Timeout;
    case Errno::Inval:
    case Errno::BadF:
    case Errno::NotDir:
    case Errno::IsDir:
    case Errno::Range:
    case Errno::Dom:
        return Status::Invalid;
    default:
        return Status::Protocol;
    }
}

[[nodiscard]] constexpr bool is_capability_error(Errno e) noexcept {
    return e == Errno::CapMode || e == Errno::NotCapable;
}

struct ErrnoName {
    Errno       code;
    const char* macro;
};

inline constexpr ErrnoName kErrnoTable[] = {
    {Errno::Ok, "OK"}, {Errno::Perm, "EPERM"}, {Errno::NoEnt, "ENOENT"},
    {Errno::Srch, "ESRCH"}, {Errno::Intr, "EINTR"}, {Errno::NoMem, "ENOMEM"},
    {Errno::Acces, "EACCES"}, {Errno::Fault, "EFAULT"}, {Errno::Busy, "EBUSY"},
    {Errno::Exist, "EEXIST"}, {Errno::Inval, "EINVAL"}, {Errno::Again, "EAGAIN"},
    {Errno::TimedOut, "ETIMEDOUT"}, {Errno::CapMode, "ECAPMODE"},
    {Errno::NotCapable, "ENOTCAPABLE"},
};

[[nodiscard]] inline unsigned errno_table_size() noexcept {
    return static_cast<unsigned>(sizeof(kErrnoTable) / sizeof(kErrnoTable[0]));
}

[[nodiscard]] inline Result<Errno> from_errno_int(int e) noexcept {
    for (const auto& row : kErrnoTable) {
        if (static_cast<int>(row.code) == e) {
            return result_ok(row.code);
        }
    }
    return result_err<Errno>(Status::NotFound);
}

} // namespace pbsd::kernel::errno_ns
