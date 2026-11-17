"""
Formal equivalence verifier, tautology detector, and SAT checker via exhaustive state exploration.
"""

from __future__ import annotations
from dataclasses import dataclass
from ast_nodes import BooleanExpr, Environment
from evaluator import evaluate, get_variables
from truth_table import generate_valuations

@dataclass(frozen=True, slots=True)
class CounterExample:
    environment: Environment
    result_a: bool
    result_b: bool

@dataclass(frozen=True, slots=True)
class VerificationResult:
    is_equivalent: bool
    total_checked: int
    counter_example: CounterExample | None = None

def verify_equivalence(expr_a: BooleanExpr, expr_b: BooleanExpr) -> VerificationResult:
    """
    Formally verifies whether two Boolean expressions are logically equivalent
    across all possible input permutations (exhaustive model checking).
    """
    vars_a = get_variables(expr_a)
    vars_b = get_variables(expr_b)
    all_vars = sorted(set(vars_a).union(vars_b))

    valuations = generate_valuations(all_vars)

    for env in valuations:
        res_a = evaluate(expr_a, env)
        res_b = evaluate(expr_b, env)

        if res_a != res_b:
            return VerificationResult(
                is_equivalent=False,
                total_checked=len(valuations),
                counter_example=CounterExample(
                    environment=env,
                    result_a=res_a,
                    result_b=res_b,
                ),
            )

    return VerificationResult(
        is_equivalent=True,
        total_checked=len(valuations),
    )

def is_tautology(expr: BooleanExpr) -> bool:
    """
    Checks if an expression is a Tautology (evaluates to True under all possible valuations).
    """
    variables = get_variables(expr)
    return all(evaluate(expr, env) for env in generate_valuations(variables))

def is_contradiction(expr: BooleanExpr) -> bool:
    """
    Checks if an expression is a Contradiction (evaluates to False under all possible valuations).
    """
    variables = get_variables(expr)
    return not any(evaluate(expr, env) for env in generate_valuations(variables))

def is_satisfiable(expr: BooleanExpr) -> bool:
    """
    Checks if an expression is Satisfiable (evaluates to True for at least one valuation).
    """
    variables = get_variables(expr)
    return any(evaluate(expr, env) for env in generate_valuations(variables))
