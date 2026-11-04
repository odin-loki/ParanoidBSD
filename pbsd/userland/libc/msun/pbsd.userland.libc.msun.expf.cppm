module;
export module pbsd.userland.libc.msun.expf;
import pbsd.userland.msun.expf;
export namespace pbsd::userland::libc::msun {
[[nodiscard]] inline float expf(float x) noexcept {
    return pbsd::userland::msun::expf_val(x);
}
}