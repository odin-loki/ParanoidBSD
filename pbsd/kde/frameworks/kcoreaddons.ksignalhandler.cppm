module;

#ifndef _WIN32
#include <cerrno>
#include <fcntl.h>
#include <signal.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

export module pbsd.kde.frameworks.kcoreaddons.ksignalhandler;

import pbsd.core;

/// Wave 3 — Unix signal socketpair bridge (from KSignalHandler).
/// Upstream: kde/frameworks/kcoreaddons/src/lib/util/ksignalhandler.cpp
export namespace pbsd::kde::frameworks::kcoreaddons::ksignalhandler {

inline constexpr unsigned kMaxSignals = 32;

struct SignalPipe {
    int read_fd{-1};
    int write_fd{-1};
    int registered[kMaxSignals]{};
    unsigned registered_count{0};
};

namespace detail {

#ifndef _WIN32

[[nodiscard]] inline Status create_pipe(SignalPipe& pipe) noexcept {
    int fds[2]{-1, -1};
    if (::socketpair(AF_UNIX, SOCK_STREAM, 0, fds) != 0) {
        return Status::Protocol;
    }
    fcntl(fds[0], F_SETFD, FD_CLOEXEC);
    fcntl(fds[1], F_SETFD, FD_CLOEXEC);
    pipe.read_fd = fds[0];
    pipe.write_fd = fds[1];
    return Status::Ok;
}

[[nodiscard]] inline Status write_signal(SignalPipe& pipe, int sig) noexcept {
    if (pipe.write_fd < 0) {
        return Status::Invalid;
    }
    const ssize_t ret = ::write(pipe.write_fd, &sig, sizeof(sig));
    return ret == sizeof(sig) ? Status::Ok : Status::Protocol;
}

[[nodiscard]] inline Result<int> read_signal(SignalPipe& pipe) noexcept {
    if (pipe.read_fd < 0) {
        return {Status::Invalid, 0};
    }
    int sig = 0;
    const ssize_t ret = ::read(pipe.read_fd, &sig, sizeof(sig));
    if (ret != sizeof(sig)) {
        return {Status::Protocol, 0};
    }
    return {Status::Ok, sig};
}

#else

[[nodiscard]] inline Status create_pipe(SignalPipe&) noexcept { return Status::NotSupported; }
[[nodiscard]] inline Status write_signal(SignalPipe&, int) noexcept { return Status::NotSupported; }
[[nodiscard]] inline Result<int> read_signal(SignalPipe&) noexcept { return {Status::NotSupported, 0}; }

#endif

} // namespace detail

[[nodiscard]] inline Status init(SignalPipe& pipe) noexcept {
    pipe.read_fd = -1;
    pipe.write_fd = -1;
    pipe.registered_count = 0;
    return detail::create_pipe(pipe);
}

[[nodiscard]] inline Status watch_signal(SignalPipe& pipe, int sig) noexcept {
#ifndef _WIN32
    if (pipe.registered_count >= kMaxSignals) {
        return Status::NoMemory;
    }
    pipe.registered[pipe.registered_count++] = sig;
    ::signal(sig, SIG_IGN); // nucleus stub — hosted installs real handler
    return Status::Ok;
#else
    (void)pipe;
    (void)sig;
    return Status::NotSupported;
#endif
}

[[nodiscard]] inline const char* upstream_path() noexcept {
    return "kde/frameworks/kcoreaddons/src/lib/util/ksignalhandler.cpp";
}

} // namespace pbsd::kde::frameworks::kcoreaddons::ksignalhandler
