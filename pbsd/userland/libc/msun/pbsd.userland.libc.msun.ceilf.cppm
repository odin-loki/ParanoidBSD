module;
export module pbsd.userland.libc.msun.ceilf;
import pbsd.userland.msun.ceilf;
export namespace pbsd::userland::libc::msun {
[[nodiscard]] inline float ceilf(float x) noexcept {
    return pbsd::userland::msun::ceilf(x);
}
}