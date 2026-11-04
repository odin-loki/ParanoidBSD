module;
export module pbsd.userland.libc.msun.cosf;
import pbsd.userland.msun.cosf;
export namespace pbsd::userland::libc::msun {
[[nodiscard]] inline float cosf(float x) noexcept {
    return pbsd::userland::msun::cosf_val(x);
}
}