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
    MacroObjectConstexprPass,
    MacroRangeMaskPass,
    RegionLifetimePass,
    SnprintfLiteralPass,
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


def passes_for_tiers(tiers: set[int] | None = None) -> list[Pass]:
    all_p = register_all_passes()
    if tiers is None:
        return all_p
    return [p for p in all_p if p.tier in tiers]
