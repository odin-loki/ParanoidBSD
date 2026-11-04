module;
export module pbsd.userland.libc.msun.roundf;
import pbsd.userland.msun.roundf;
export namespace pbsd::userland::libc::msun {
[[nodiscard]] inline float roundf(float x) noexcept {
    return pbsd::userland::msun::roundf(x);
}
}