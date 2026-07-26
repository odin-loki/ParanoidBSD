"""Union-Find with lattice join — CCured-style pointer kind inference."""
from __future__ import annotations

from enum import IntEnum


class PtrKind(IntEnum):
    SAFE = 0  # no arithmetic, no cast
    SEQ = 1  # arithmetic / needs bounds
    WILD = 2  # cast / punned


class UnionFindLattice:
    """Near-linear unification; kind(x) = join over equivalence class."""

    def __init__(self) -> None:
        self.parent: dict[str, str] = {}
        self.rank: dict[str, int] = {}
        self.kind: dict[str, PtrKind] = {}

    def add(self, x: str, k: PtrKind = PtrKind.SAFE) -> None:
        if x not in self.parent:
            self.parent[x] = x
            self.rank[x] = 0
            self.kind[x] = k
        else:
            self.kind[self.find(x)] = max(self.kind[self.find(x)], k)

    def find(self, x: str) -> str:
        if x not in self.parent:
            self.add(x)
        while self.parent[x] != x:
            self.parent[x] = self.parent[self.parent[x]]
            x = self.parent[x]
        return x

    def union(self, a: str, b: str) -> None:
        ra, rb = self.find(a), self.find(b)
        if ra == rb:
            return
        ka, kb = self.kind[ra], self.kind[rb]
        joined = max(ka, kb)
        if self.rank[ra] < self.rank[rb]:
            self.parent[ra] = rb
            self.kind[rb] = joined
        elif self.rank[ra] > self.rank[rb]:
            self.parent[rb] = ra
            self.kind[ra] = joined
        else:
            self.parent[rb] = ra
            self.rank[ra] += 1
            self.kind[ra] = joined

    def get_kind(self, x: str) -> PtrKind:
        return self.kind[self.find(x)]

    def raise_kind(self, x: str, k: PtrKind) -> None:
        r = self.find(x)
        self.kind[r] = max(self.kind[r], k)


class Steensgaard:
    """Unification-based points-to (Steensgaard): near-linear, coarse.

    Each variable has a location set representative. Assignment p = q unifies
    pts(p) with pts(q). *p = q / p = *q also unify through a 'cell' node.
    """

    def __init__(self) -> None:
        self.uf = UnionFindLattice()
        self.cell: dict[str, str] = {}  # loc-rep → pointed-to loc-rep name
        self.escaped: set[str] = set()

    def _loc(self, name: str) -> str:
        return f"loc:{name}"

    def _ensure_cell(self, loc_rep: str) -> str:
        loc_rep = self.uf.find(loc_rep)
        if loc_rep not in self.cell:
            cell = f"cell:{loc_rep}"
            self.uf.add(cell)
            self.cell[loc_rep] = cell
        return self.uf.find(self.cell[loc_rep])

    def assign(self, dst: str, src: str) -> None:
        """dst = src (pointer copy)."""
        self.uf.add(self._loc(dst))
        self.uf.add(self._loc(src))
        self.uf.union(self._loc(dst), self._loc(src))

    def store(self, ptr: str, val: str) -> None:
        """*ptr = val"""
        self.uf.add(self._loc(ptr))
        self.uf.add(self._loc(val))
        cell = self._ensure_cell(self.uf.find(self._loc(ptr)))
        self.uf.union(cell, self._loc(val))

    def load(self, dst: str, ptr: str) -> None:
        """dst = *ptr"""
        self.uf.add(self._loc(dst))
        self.uf.add(self._loc(ptr))
        cell = self._ensure_cell(self.uf.find(self._loc(ptr)))
        self.uf.union(self._loc(dst), cell)

    def mark_escape(self, name: str) -> None:
        rep = self.uf.find(self._loc(name))
        self.escaped.add(rep)
        # escape the pointed-to cell too
        if rep in self.cell:
            self.escaped.add(self.uf.find(self.cell[rep]))

    def escapes(self, name: str) -> bool:
        return self.uf.find(self._loc(name)) in self.escaped

    def may_alias(self, a: str, b: str) -> bool:
        return self.uf.find(self._loc(a)) == self.uf.find(self._loc(b))


class NullLattice(IntEnum):
    NEVER_NULL = 0
    MAYBE_NULL = 1
    DEFINITE_NULL = 2


class NullabilityFixpoint:
    """Monotone nullability over a sequence of statements (cheap CFG approx)."""

    def __init__(self) -> None:
        self.state: dict[str, NullLattice] = {}

    def get(self, x: str) -> NullLattice:
        return self.state.get(x, NullLattice.MAYBE_NULL)

    def set_join(self, x: str, v: NullLattice) -> None:
        cur = self.get(x)
        self.state[x] = max(cur, v)

    def set_meet_after_check(self, x: str) -> None:
        """After `if (x == null) return`, on fallthrough x is NEVER_NULL."""
        self.state[x] = NullLattice.NEVER_NULL

    def assign_null(self, x: str) -> None:
        self.state[x] = NullLattice.DEFINITE_NULL

    def assign_alloc(self, x: str) -> None:
        # malloc result is maybe-null until checked
        self.state[x] = NullLattice.MAYBE_NULL

    def assign_addr(self, x: str) -> None:
        self.state[x] = NullLattice.NEVER_NULL
