"""Refusal-log schema — primary output of every pass (todo.md Tier 0)."""
from __future__ import annotations

from dataclasses import asdict, dataclass
from typing import Any


REASON_CODES = frozenset(
    {
        "VOID_PTR_ASSIGN",
        "CPP_KEYWORD",
        "REGISTER",
        "KR_DEFINITION",
        "IMPLICIT_INT",
        "ENUM_ARITHMETIC",
        "STRING_LITERAL_CONST",
        "DESIGNATED_INIT",
        "COMPOUND_LITERAL",
        "FLEXIBLE_ARRAY",
        "NESTED_STRUCT_TAG",
        "TENTATIVE_DEFINITION",
        "RESTRICT",
        "C11_TYPE",
        "GENERIC",
        "VLA",
        "GOTO_CROSS_INIT",
        "GOTO_CLEANUP_CANDIDATE",
        "NULL_TO_NULLPTR",
        "TYPEDEF_TO_USING",
        "TYPEDEF_COMPLEX",
        "MALLOC_RAII",
        "MALLOC_ESCAPE",
        "STACK_BUF_ARRAY",
        "CONST_CANDIDATE",
        "DEAD_STORE",
        "RANGE_FOR_CANDIDATE",
        "POINTER_KIND",
        "SPAN_CANDIDATE",
        "CALL_SITE_SPAN",
        "QUEUE_H_SITE",
        "NULLABILITY",
        "BIT_CAST_CANDIDATE",
        "PURITY",
        "GLOBAL_CLUSTER",
        "LOCK_DISCIPLINE",
        "ERROR_CODE_EXPECTED",
        "MACRO_DIVERGENT",
        "MACRO_OBJECT",
        "MACRO_CONSTEXPR",
        "FN_PTR_STRUCT",
        "CALLBACK_VOID_CTX",
        "STR_FORMAT_CANDIDATE",
        "ANDERSEN_ESCAPE",
        "ANDERSEN_OWNED",
        "LIFETIME_FAIL",
        "LIFETIME_OWN",
        "SYSCALL_TRACE",
        "IR_MISMATCH",
        "IR_SKIP",
        "PRECONDITION_FAIL",
        "PARSE_FAIL",
        "MODEL_QUEUE",
    }
)


@dataclass
class Refusal:
    file: str
    line: int
    col: int
    pass_name: str
    reason_code: str
    enclosing_function: str
    snippet: str

    def to_dict(self) -> dict[str, Any]:
        d = asdict(self)
        d["pass"] = d.pop("pass_name")
        return d


@dataclass
class Edit:
    pass_name: str
    description: str
    line: int
    old: str
    new: str


@dataclass
class PassResult:
    text: str
    refusals: list[Refusal]
    edits: list[Edit]

    @staticmethod
    def unchanged(text: str) -> PassResult:
        return PassResult(text=text, refusals=[], edits=[])


@dataclass
class StageEvidence:
    """Appendix A stage bits for a Port Record (B spec, C sanitizers, D tests, E envelope, F port, G oracle)."""

    B: str = "pending"
    C: str = "pending"
    D: str = "pending"
    E: str = "pending"
    F: str = "pending"
    G: str = "pending"

    def to_dict(self) -> dict[str, Any]:
        return asdict(self)


@dataclass
class PortRecord:
    """Stage H bookkeeping written by tools/pbsd_agent_port.py."""

    source: str
    model_used: str
    escalation_trail: list[dict[str, Any]]
    stage_evidence: dict[str, Any]
    tokens_in: int
    tokens_out: int
    est_cost_usd: float
    status: str
    cache_hit_tokens: int = 0
    cache_miss_tokens: int = 0

    def to_dict(self) -> dict[str, Any]:
        return asdict(self)
