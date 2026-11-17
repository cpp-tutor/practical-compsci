"""
Execution harness validating imperative-to-expression desugaring,
guarded short-circuiting, and algebraic branch reduction.
"""

from __future__ import annotations
from ast_nodes import Expr, LiteralExpr, VariableExpr, BinaryExpr, LetExpr, IfExpr, StatementIfNode, Branch
from desugar import desugar_statement_if
from evaluator import GuardedEvaluator
from optimizer import BranchOptimizer

def format_ast(expr: Expr) -> str:
    """Pretty-prints an AST expression into a compact string representation."""
    match expr:
        case LiteralExpr(value):
            return str(value)
        case VariableExpr(name):
            return name
        case BinaryExpr(op, left, right):
            return f"({format_ast(left)} {op} {format_ast(right)})"
        case LetExpr(name, initializer, body):
            return f"(let {name} = {format_ast(initializer)} in {format_ast(body)})"
        case IfExpr(condition, then_branch, else_branch):
            return (
                f"({format_ast(condition)} ? "
                f"{format_ast(then_branch)} : {format_ast(else_branch)})"
            )
        case StatementIfNode(target_var, _, _, _):
            return f"[stmt-if on {target_var}]"

def banner(title: str) -> None:
    print('\n' + '=' * 74)
    print(f"  {title.upper()}")
    print('=' * 74)

def run_demonstrations() -> None:
    # -------------------------------------------------------------
    # Test 1: Desugaring Imperative Statement-If to Expression
    # -------------------------------------------------------------
    banner('1. Statement Desugaring to Pure Conditional Expression')

    # Statements: if orderTotal > 100
    #               discount = 20
    #             else
    #               discount = 5
    #             endif
    #             discount = discount * 2
    imperative_node = Branch.stmt_if(
        'discount',
        Branch.bin('>', Branch.var('orderTotal'), Branch.lit(100)),
        Branch.lit(20),
        Branch.lit(5),
    )

    continuation = Branch.bin('*', Branch.var('discount'), Branch.lit(2))
    desugared_program = desugar_statement_if(imperative_node, continuation)

    print('Desugared Expression:')
    print(f"  {format_ast(desugared_program)}")

    evaluator = GuardedEvaluator()
    res1 = evaluator.evaluate(desugared_program, {'orderTotal': 150})
    print(f"Evaluated with orderTotal = 150: Result = {res1}")

    # -------------------------------------------------------------
    # Test 2: Safe Guarded Execution and Short-Circuit Verification
    # -------------------------------------------------------------
    banner('2. Safe Guarded Execution (Division-by-Zero Protection)')

    # Expression: (divisor == 0) ? 0 : (100 / divisor)
    safe_div_program = Branch.if_(
        Branch.bin('==', Branch.var('divisor'), Branch.lit(0)),
        Branch.lit(0),
        Branch.bin('/', Branch.lit(100), Branch.var('divisor')),
    )

    print(f"Program: {format_ast(safe_div_program)}")
    safe_eval = GuardedEvaluator()
    safe_res = safe_eval.evaluate(safe_div_program, {'divisor': 0})
    print(
        f"Evaluated with divisor = 0: Result = {safe_res} "
        f"(No division-by-zero error raised)"
    )
    print(f"Branch History: {safe_eval.get_branch_history()}")

    # -------------------------------------------------------------
    # Test 3: Algebraic Branch Optimisation and Tree Rewriting
    # -------------------------------------------------------------
    banner('3. Symbolic Algebraic Branch Optimisation')

    # Nested Redundant Program:
    # if (true) then (
    #   if (x > 5) then 42 else 42
    # ) else (
    #   999
    # )
    redundant_tree = Branch.if_(
        Branch.lit(True),
        Branch.if_(
            Branch.bin('>', Branch.var('x'), Branch.lit(5)),
            Branch.lit(42),
            Branch.lit(42),
        ),
        Branch.lit(999),
    )

    print(f"Original Redundant Tree:\n  {format_ast(redundant_tree)}")

    optimizer = BranchOptimizer()
    optimized_ast = optimizer.optimize_to_fixed_point(redundant_tree)
    metrics = optimizer.get_metrics()

    print(f"\nOptimised Tree:\n  {format_ast(optimized_ast)}")
    print("\nOptimization Metrics:")
    print(f"  Nodes Eliminated: {metrics.nodes_eliminated}")
    print(f"  Rules Applied:    {' -> '.join(metrics.rules_applied)}")

if __name__ == '__main__':
    run_demonstrations()