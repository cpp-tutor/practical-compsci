"""
Recursive evaluator, variable collector, and mathematical formatter for Boolean ASTs.
"""

from __future__ import annotations
from ast_nodes import BooleanExpr, ConstantExpr, VariableExpr, NotExpr, BinaryExpr, Environment

def evaluate(expr: BooleanExpr, env: Environment) -> bool:
    """
    Evaluates a Boolean AST expression recursively under a given variable environment.
    """
    match expr:
        case ConstantExpr(val):
            return val

        case VariableExpr(name):
            if name not in env:
                raise KeyError(f"Unbound variable '{name}' in evaluation environment.")
            return env[name]

        case NotExpr(operand):
            return not evaluate(operand, env)

        case BinaryExpr('and', left, right):
            return evaluate(left, env) and evaluate(right, env)

        case BinaryExpr('or', left, right):
            return evaluate(left, env) or evaluate(right, env)

        case BinaryExpr('xor', left, right):
            l_val = evaluate(left, env)
            r_val = evaluate(right, env)
            return (l_val and not r_val) or (not l_val and r_val)

        case BinaryExpr('nand', left, right):
            return not (evaluate(left, env) and evaluate(right, env))

        case BinaryExpr('nor', left, right):
            return not (evaluate(left, env) or evaluate(right, env))

        case BinaryExpr('xnor', left, right):
            return evaluate(left, env) == evaluate(right, env)

        case _:
            raise TypeError(f"Unknown expression node encountered: {type(expr)}")

def get_variables(expr: BooleanExpr) -> list[str]:
    """
    Collects all unique variable names present in an AST expression, returned in sorted order.
    """
    vars_found: set[str] = set()

    def traverse(node: BooleanExpr) -> None:
        match node:
            case VariableExpr(name):
                vars_found.add(name)
            case NotExpr(operand):
                traverse(operand)
            case BinaryExpr(_, left, right):
                traverse(left)
                traverse(right)
            case ConstantExpr(_):
                pass

    traverse(expr)
    return sorted(vars_found)

def format_expression(expr: BooleanExpr) -> str:
    """
    Formats a Boolean AST into a human-readable mathematical string representation.
    """
    match expr:
        case ConstantExpr(val):
            return '1' if val else '0'
        case VariableExpr(name):
            return name
        case NotExpr(operand):
            return f"¬({format_expression(operand)})"
        case BinaryExpr('and', left, right):
            return f"({format_expression(left)} ∧ {format_expression(right)})"
        case BinaryExpr('or', left, right):
            return f"({format_expression(left)} ∨ {format_expression(right)})"
        case BinaryExpr('xor', left, right):
            return f"({format_expression(left)} ⊕ {format_expression(right)})"
        case BinaryExpr('nand', left, right):
            return f"({format_expression(left)} ↑ {format_expression(right)})"
        case BinaryExpr('nor', left, right):
            return f"({format_expression(left)} ↓ {format_expression(right)})"
        case BinaryExpr('xnor', left, right):
            return f"({format_expression(left)} ⊙ {format_expression(right)})"
