"""
Abstract Syntax Tree (AST) definitions for symbolic Boolean logic expressions.
"""

from __future__ import annotations
from dataclasses import dataclass
from typing import Literal, Mapping, TypeAlias

# Variable assignment environment mapping variable names to Boolean values
Environment: TypeAlias = Mapping[str, bool]

BinaryOpKind: TypeAlias = Literal['and', 'or', 'xor', 'nand', 'nor', 'xnor']
ExprKind: TypeAlias = Literal['constant', 'variable', 'not', BinaryOpKind]

@dataclass(frozen=True, slots=True)
class ConstantExpr:
    value: bool
    kind: Literal['constant'] = 'constant'

@dataclass(frozen=True, slots=True)
class VariableExpr:
    name: str
    kind: Literal['variable'] = 'variable'

@dataclass(frozen=True, slots=True)
class NotExpr:
    operand: BooleanExpr
    kind: Literal['not'] = 'not'

@dataclass(frozen=True, slots=True)
class BinaryExpr:
    kind: BinaryOpKind
    left: BooleanExpr
    right: BooleanExpr

BooleanExpr: TypeAlias = ConstantExpr | VariableExpr | NotExpr | BinaryExpr

class Logic:
    """Smart constructors (factories) for Boolean AST nodes."""

    @staticmethod
    def const(value: bool) -> ConstantExpr:
        return ConstantExpr(value)

    @staticmethod
    def var(name: str) -> VariableExpr:
        return VariableExpr(name)

    @staticmethod
    def not_(operand: BooleanExpr) -> NotExpr:
        return NotExpr(operand)

    @staticmethod
    def and_(left: BooleanExpr, right: BooleanExpr) -> BinaryExpr:
        return BinaryExpr('and', left, right)

    @staticmethod
    def or_(left: BooleanExpr, right: BooleanExpr) -> BinaryExpr:
        return BinaryExpr('or', left, right)

    @staticmethod
    def xor(left: BooleanExpr, right: BooleanExpr) -> BinaryExpr:
        return BinaryExpr('xor', left, right)

    @staticmethod
    def nand(left: BooleanExpr, right: BooleanExpr) -> BinaryExpr:
        return BinaryExpr('nand', left, right)

    @staticmethod
    def nor(left: BooleanExpr, right: BooleanExpr) -> BinaryExpr:
        return BinaryExpr('nor', left, right)

    @staticmethod
    def xnor(left: BooleanExpr, right: BooleanExpr) -> BinaryExpr:
        return BinaryExpr('xnor', left, right)
