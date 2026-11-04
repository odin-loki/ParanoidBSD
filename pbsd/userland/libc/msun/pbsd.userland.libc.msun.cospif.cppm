module;
export module pbsd.userland.libc.msun.cospif;
import pbsd.userland.msun.cospif;
export namespace pbsd::userland::libc::msun {
[[nodiscard]] inline float cospif(float x) noexcept {
    return pbsd::userland::msun::cospif_val(x);
}
}
