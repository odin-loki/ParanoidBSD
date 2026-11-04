module;
export module pbsd.userland.libc.msun.coshf;
import pbsd.userland.msun.coshf;
export namespace pbsd::userland::libc::msun {
[[nodiscard]] inline float coshf(float x) noexcept {
    return pbsd::userland::msun::coshf_val(x);
}
}