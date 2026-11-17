"""
Runtime representation of lexical environments, parent pointer chains,
and first-class closures.
"""

from __future__ import annotations
from typing import TypeAlias
from ast_nodes import Expr, PrimitiveValue

class Closure:
    """
    Represents a runtime Closure pairing formal parameters and function body
    with its captured lexical environment frame.
    """

    def __init__(
        self,
        parameters: tuple[str, ...],
        body: Expr,
        captured_environment: Environment,
        name_tag: str | None = None,
    ) -> None:
        self.parameters: tuple[str, ...] = parameters
        self.body: Expr = body
        self.captured_environment: Environment = captured_environment
        self.name_tag: str | None = name_tag

    def __str__(self) -> str:
        name = f"[Closure: {self.name_tag}]" if self.name_tag else "[Anonymous Closure]"
        return f"{name}({', '.join(self.parameters)})"

    def __repr__(self) -> str:
        return self.__str__()

RuntimeValue: TypeAlias = PrimitiveValue | Closure

class Environment:
    """
    Scoped lexical environment frame with parent pointer hierarchy.
    """

    def __init__(self, parent: Environment | None = None) -> None:
        self._store: dict[str, RuntimeValue] = {}
        self.parent: Environment | None = parent

    def define(self, name: str, value: RuntimeValue) -> None:
        self._store[name] = value

    def lookup(self, name: str) -> RuntimeValue:
        if name in self._store:
            return self._store[name]
        if self.parent is not None:
            return self.parent.lookup(name)
        raise NameError(f"Runtime Error: Unbound identifier '{name}'.")

    def snapshot(self) -> dict[str, str]:
        """Produces a snapshot mapping of all visible bindings."""
        result: dict[str, str] = self.parent.snapshot() if self.parent is not None else {}
        for k, v in self._store.items():
            result[k] = str(v)
        return result