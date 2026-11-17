"""
Abstract Syntax Tree (AST) definitions for functions, lambdas, and invocations.
"""

from __future__ import annotations
from dataclasses import dataclass
from typing import Literal, Sequence, TypeAlias

PrimitiveValue: TypeAlias = int | float | bool | str

BinaryOp: TypeAlias = Literal['+', '-', '*', '/', '==', '<', '&&']

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
    op: BinaryOp
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
class IfExpr:
    condition: Expr
    then_branch: Expr
    else_branch: Expr
    kind: Literal['if'] = 'if'

@dataclass(frozen=True, slots=True)
class LambdaExpr:
    """
    Anonymous function declaration: (p1, p2, ...) => body
    """
    parameters: tuple[str, ...]
    body: Expr
    kind: Literal['lambda'] = 'lambda'

@dataclass(frozen=True, slots=True)
class CallExpr:
    """
    Function invocation: callee(arg1, arg2, ...)
    """
    callee: Expr
    arguments: tuple[Expr, ...]
    kind: Literal['call'] = 'call'

Expr: TypeAlias = (
    LiteralExpr
    | VariableExpr
    | BinaryExpr
    | LetExpr
    | IfExpr
    | LambdaExpr
    | CallExpr
)

class Fn:
    """Smart constructors (factories) for Functional AST nodes."""

    @staticmethod
    def lit(value: PrimitiveValue) -> LiteralExpr:
        return LiteralExpr(value)

    @staticmethod
    def var(name: str) -> VariableExpr:
        return VariableExpr(name)

    @staticmethod
    def bin(op: BinaryOp, left: Expr, right: Expr) -> BinaryExpr:
        return BinaryExpr(op, left, right)

    @staticmethod
    def let(name: str, initializer: Expr, body: Expr) -> LetExpr:
        return LetExpr(name, initializer, body)

    @staticmethod
    def if_(condition: Expr, then_branch: Expr, else_branch: Expr) -> IfExpr:
        return IfExpr(condition, then_branch, else_branch)

    @staticmethod
    def lambda_(parameters: Sequence[str], body: Expr) -> LambdaExpr:
        return LambdaExpr(tuple(parameters), body)

    @staticmethod
    def call(callee: Expr, *args: Expr) -> CallExpr:
        return CallExpr(callee, tuple(args))