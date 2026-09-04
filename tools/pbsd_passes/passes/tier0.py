# SPDX-License-Identifier: AGPL-3.0-or-later
# SPDX-FileCopyrightText: 2026 Odin Loch <odin.loch@outlook.com.au>
"""Tier 0 — infrastructure markers (real work lives in runner/compile_db/ir_oracle)."""
from __future__ import annotations

from ..schema import PassResult
from ..unit import TranslationUnit
from .base import Pass


class Tier0MarkerPass(Pass):
    name = "tier0_marker"
    tier = 0

    def apply(self, unit: TranslationUnit) -> PassResult:
        # No source rewrite; runner handles compile_commands / corpus / IR.
        unit.meta["tier0_seen"] = True
        return PassResult.unchanged(unit.text)


TIER0_PASSES: list[Pass] = [Tier0MarkerPass()]
