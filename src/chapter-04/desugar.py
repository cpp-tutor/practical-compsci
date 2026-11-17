"""
Transpiler lowering imperative statement-level branch blocks into pure
functional expressions.
"""

from __future__ import annotations
from ast_nodes import Expr, StatementIfNode, Branch

def desugar_statement_if(stmt: StatementIfNode, continuation_body: Expr) -> Expr:
    """
    Transforms imperative conditional statements into equivalent functional let-bindings
    wrapping pure conditional expressions.

    Example transformation:
        let x;
        if (c) { x = A; } else { x = B; }
        rest...

    Becomes:
        let x = (c ? A : B) in rest...
    """
    conditional_expression = Branch.if_(
        stmt.condition,
        stmt.then_assign,
        stmt.else_assign,
    )

    return Branch.let(
        stmt.target_variable,
        conditional_expression,
        continuation_body,
    )