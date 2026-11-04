# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Pass package exports."""
from __future__ import annotations

from .base import Pass
from .promote_malloc_span import PROMOTE_MALLOC_SPAN_PASSES
from .promote_mop_up import MOP_UP_PASSES
from .promote_null_bitcast_goto import NULL_BITCAST_GOTO_PASSES
from .promote_proposals import PROPOSAL_PASSES
from .promote_queue_ops import PROMOTE_QUEUE_OPS_PASSES
from .promote_refusals import REFUSAL_PROMOTE_PASSES
from .promote_remaining import (
    Alive2OracleStubPass,
    AndersenEscapePass,
    CallbackCtxPass,
    FnPtrStructPass,
    FuzzOracleStubPass,
    MacroAntiUnificationPass,
    MacroFunctionConstexprPass,
    MacroObjectConstexprPass,
    MacroRangeMaskPass,
    RegionLifetimePass,
    SnprintfLiteralPass,
    StrcpyLiteralPass,
    StrToStringViewPass,
    SyscallTraceStubPass,
    TentativeDefinitionPass,
)
from .tier0 import TIER0_PASSES
from .tier1 import TIER1_PASSES
from .tier2 import TIER2_PASSES
from .tier3 import TIER3_PASSES
from .tier3_maths import MATHS_PASSES
from .tier4 import TIER4_PASSES


def register_all_passes() -> list[Pass]:
    return [
        *TIER0_PASSES,
        MacroRangeMaskPass(),
        *TIER1_PASSES,
        TentativeDefinitionPass(),
        MacroObjectConstexprPass(),
        MacroFunctionConstexprPass(),
        *REFUSAL_PROMOTE_PASSES,
        *PROMOTE_MALLOC_SPAN_PASSES,
        *NULL_BITCAST_GOTO_PASSES,
        *MATHS_PASSES,
        *PROMOTE_QUEUE_OPS_PASSES,
        *MOP_UP_PASSES,
        *PROPOSAL_PASSES,
        *TIER2_PASSES,
        StrToStringViewPass(),
        SnprintfLiteralPass(),
        StrcpyLiteralPass(),
        *TIER3_PASSES,
        AndersenEscapePass(),
        RegionLifetimePass(),
        MacroAntiUnificationPass(),
        FnPtrStructPass(),
        CallbackCtxPass(),
        *TIER4_PASSES,
        Alive2OracleStubPass(),
        FuzzOracleStubPass(),
        SyscallTraceStubPass(),
    ]


# Passes whose rewrites are safe enough to commit, as opposed to propose.
#
# The criterion is narrow on purpose: a local, syntactic substitution that
# does not reinterpret a pointer as a buffer, change ownership or
# allocation, or restructure control flow. NULL -> nullptr qualifies;
# foo(buf, n) -> foo(std::span(buf, n)) does not, because it asserts that
# two arguments are a pointer and its length.
#
# This split is not a guess. Measured over usr.sbin, bin, sbin and usr.bin,
# null_to_nullptr and const_param_rewrite alone are 68% of all edits, the
# set below is about 78%, and every one of the 455 corrupted staged files
# came from the span/RAII family that is excluded. Meanwhile lib/msun, the
# only place anything has been verified, averages 0.30 edits per file
# against 35.7 in that sample - the 69 verified ports verify largely
# because almost nothing was done to them.
#
# Excluded passes are not disabled. They still run in a normal invocation,
# and they still write proposals.jsonl, which is where cxx23-port-master-
# plan.md 8.3 says this class of rewrite belongs: matcher finds candidates,
# human or stronger model confirms.
#
# Widen this set against oracle results, not intuition.
SAFE_PASS_NAMES = frozenset({
    "macro_range_mask",
    "tier0_marker",
    "c11_to_cxx",
    "cpp_keyword_rename",
    "designated_init_cxx20",
    "flexible_array_refuse",
    "generic_refuse",
    "knr_reject",
    "kr_definition_fix",
    "macro_function_constexpr",
    "macro_object_constexpr",
    "nested_struct_tag_refuse",
    "null_to_nullptr",
    "register_remove",
    "restrict_to_underscore",
    "string_literal_const",
    "tentative_definition",
    "typedef_to_using",
    "const_param_rewrite",
    "dead_store_propose_only",
    "global_cluster_propose_only",
    "purity_propose_only",
    "lock_discipline_propose",
})


def passes_for_tiers(
    tiers: set[int] | None = None,
    safe_only: bool = False,
) -> list[Pass]:
    all_p = register_all_passes()
    if tiers is not None:
        all_p = [p for p in all_p if p.tier in tiers]
    if safe_only:
        all_p = [p for p in all_p if p.name in SAFE_PASS_NAMES]
    return all_p
