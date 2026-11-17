"""
Abstract Syntax Tree (AST) definitions for branching, conditional expressions,
and imperative statement blocks.
"""

from __future__ import annotations
from dataclasses import dataclass
from typing import Literal, TypeAlias

PrimitiveValue: TypeAlias = int | float | bool | str

BinaryOp: TypeAlias = Literal['+', '-', '*', '/', '==', '!=', '<', '<=', '>', '>=', '&&', '||']

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
    """
    Functional Conditional Expression: condition ? then_branch : else_branch
    """
    condition: Expr
    then_branch: Expr
    else_branch: Expr
    kind: Literal['if'] = 'if'

@dataclass(frozen=True, slots=True)
class StatementIfNode:
    """
    Imperative Statement-based If Block (for transpilation/desugaring experiments).
    """
    target_variable: str
    condition: Expr
    then_assign: Expr
    else_assign: Expr
    kind: Literal['statement-if'] = 'statement-if'

Expr: TypeAlias = (
    LiteralExpr
    | VariableExpr
    | BinaryExpr
    | LetExpr
    | IfExpr
    | StatementIfNode
)

class Branch:
    """Smart constructors (factories) for Branch AST nodes."""

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
    def stmt_if(
        target_variable: str,
        condition: Expr,
        then_assign: Expr,
        else_assign: Expr,
    ) -> StatementIfNode:
        return StatementIfNode(target_variable, condition, then_assign, else_assign)