"""
Abstract Syntax Tree (AST) definitions for program flow, bindings, and expressions.
"""

from __future__ import annotations
from dataclasses import dataclass
from typing import Literal, Sequence, TypeAlias

# Primitive terminal values supported by our language
PrimitiveValue: TypeAlias = int | float | bool | str

BinaryOperator: TypeAlias = Literal['+', '-', '*', '/', '==', '<', '&&', '||']

@dataclass(frozen=True, slots=True)
class LiteralExpr:
    value: PrimitiveValue
    kind: Literal['literal'] = 'literal'

@dataclass(frozen=True, slots=True)
class VariableExpr:
    name: str
    kind: Literal['variable'] = 'variable'

@dataclass(frozen=True, slots=True)
class BinaryExpr:
    op: BinaryOperator
    left: Expr
    right: Expr
    kind: Literal['binary'] = 'binary'

@dataclass(frozen=True, slots=True)
class LetExpr:
    name: str
    initializer: Expr
    body: Expr
    kind: Literal['let'] = 'let'

@dataclass(frozen=True, slots=True)
class AssignExpr:
    name: str
    value: Expr
    kind: Literal['assign'] = 'assign'

@dataclass(frozen=True, slots=True)
class SequenceExpr:
    expressions: tuple[Expr, ...]
    kind: Literal['sequence'] = 'sequence'

@dataclass(frozen=True, slots=True)
class BlockExpr:
    body: Expr
    kind: Literal['block'] = 'block'

Expr: TypeAlias = (
    LiteralExpr
    | VariableExpr
    | BinaryExpr
    | LetExpr
    | AssignExpr
    | SequenceExpr
    | BlockExpr
)

class Flow:
    """Smart constructors (factories) for Flow AST nodes."""

    @staticmethod
    def lit(value: PrimitiveValue) -> LiteralExpr:
        return LiteralExpr(value)

    @staticmethod
    def var(name: str) -> VariableExpr:
        return VariableExpr(name)

    @staticmethod
    def bin(op: BinaryOperator, left: Expr, right: Expr) -> BinaryExpr:
        return BinaryExpr(op, left, right)

    @staticmethod
    def let(name: str, initializer: Expr, body: Expr) -> LetExpr:
        return LetExpr(name, initializer, body)

    @staticmethod
    def assign(name: str, value: Expr) -> AssignExpr:
        return AssignExpr(name, value)

    @staticmethod
    def seq(*expressions: Expr) -> SequenceExpr:
        return SequenceExpr(tuple(expressions))

    @staticmethod
    def block(body: Expr) -> BlockExpr:
        return BlockExpr(body)