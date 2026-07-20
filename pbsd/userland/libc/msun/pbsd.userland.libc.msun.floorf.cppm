module;
export module pbsd.userland.libc.msun.floorf;
import pbsd.userland.msun.floorf;
export namespace pbsd::userland::libc::msun {
[[nodiscard]] inline float floorf(float x) noexcept {
    return pbsd::userland::msun::floorf(x);
}
}