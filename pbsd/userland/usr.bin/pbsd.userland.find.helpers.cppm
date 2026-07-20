module;
#include <cstddef>

export module pbsd.userland.find.helpers;

export import pbsd.core;
export import pbsd.userland.hosted;

/// Port helpers from hbsd/src/usr.bin/find/operator.c — Boolean operator tokens (-a -o !)
export namespace pbsd::userland::usr_bin::find::helpers {

enum class BoolOp : unsigned char { And, Or, Not, End };

[[nodiscard]] inline BoolOp bool_op_from_token(const char* tok) noexcept {
    if (tok == nullptr) {
        return BoolOp::End;
    }
    if (hosted::cstrcmp(tok, "-a") == 0) {
        return BoolOp::And;
    }
    if (hosted::cstrcmp(tok, "-o") == 0) {
        return BoolOp::Or;
    }
    if (hosted::cstrcmp(tok, "!") == 0 || hosted::cstrcmp(tok, "-not") == 0) {
        return BoolOp::Not;
    }
    return BoolOp::End;
}

[[nodiscard]] inline bool is_primary_token(const char* tok) noexcept {
    if (tok == nullptr || tok[0] != '-') {
        return false;
    }
    return bool_op_from_token(tok) == BoolOp::End;
}

} // namespace pbsd::userland::usr_bin::find::helpers
