"""
Truth table generation, tabular formatting, and Canonical Sum of Products (SOP) synthesis.
"""

from __future__ import annotations
from dataclasses import dataclass
from functools import reduce
from typing import Sequence
from ast_nodes import BooleanExpr, Environment, Logic
from evaluator import evaluate, get_variables, format_expression

@dataclass(frozen=True, slots=True)
class TruthTableRow:
    inputs: dict[str, bool]
    output: bool

@dataclass(frozen=True, slots=True)
class TruthTable:
    variables: tuple[str, ...]
    rows: tuple[TruthTableRow, ...]
    expression_string: str

def generate_valuations(variables: Sequence[str]) -> list[dict[str, bool]]:
    """
    Generates all 2^N input combinations for an ordered sequence of N variable names.
    """
    count = len(variables)
    total_combinations = 1 << count  # 2^N
    valuations: list[dict[str, bool]] = []

    for i in range(total_combinations):
        env: dict[str, bool] = {}
        for bit in range(count):
            var_name = variables[count - 1 - bit]
            is_set = (i & (1 << bit)) != 0
            env[var_name] = is_set
        valuations.append(env)

    return valuations

def build_truth_table(expr: BooleanExpr) -> TruthTable:
    """
    Builds a complete truth table for any arbitrary Boolean AST.
    """
    variables = tuple(get_variables(expr))
    valuations = generate_valuations(variables)

    rows = tuple(
        TruthTableRow(inputs=env, output=evaluate(expr, env))
        for env in valuations
    )

    return TruthTable(
        variables=variables,
        rows=rows,
        expression_string=format_expression(expr),
    )

def print_truth_table(table: TruthTable) -> None:
    """
    Pretty-prints a truth table to stdout with grid alignment.
    """
    print(f"\nTruth Table for: {table.expression_string}")
    header_vars = " | ".join(table.variables)
    divider = "-" * (len(header_vars) + 11)
    print(f" {header_vars} | Output")
    print(divider)

    for row in table.rows:
        input_vals = "|".join(" 1 " if row.inputs[v] else " 0 " for v in table.variables)
        out_val = "   1" if row.output else "   0"
        print(f"{input_vals}|{out_val}")

def synthesise_sop(table: TruthTable) -> BooleanExpr:
    """
    Synthesises the Canonical Sum of Products (SOP / Disjunctive Normal Form) AST from a truth table.
    """
    true_rows = [row for row in table.rows if row.output]

    if not true_rows:
        return Logic.const(False)

    minterms: list[BooleanExpr] = []
    for row in true_rows:
        literals: list[BooleanExpr] = [
            Logic.var(v) if row.inputs[v] else Logic.not_(Logic.var(v))
            for v in table.variables
        ]
        minterm = reduce(Logic.and_, literals)
        minterms.append(minterm)

    return reduce(Logic.or_, minterms)
