module;
export module pbsd.userland.libc.msun.fabsf;
import pbsd.userland.msun.fabsf;
export namespace pbsd::userland::libc::msun {
[[nodiscard]] inline float fabsf(float x) noexcept {
    return pbsd::userland::msun::fabsf_val(x);
}
}