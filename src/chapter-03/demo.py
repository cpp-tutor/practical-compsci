"""
Execution harness comparing the Dictionary Notional Machine against pure Term Rewriting.
"""

from __future__ import annotations
import json
from ast_nodes import Flow
from environment_machine import DictionaryStepper
from substitution_machine import SubstitutionStepper, format_ast

def banner(title: str) -> None:
    print('\n' + '=' * 76)
    print(f"  {title.upper()}")
    print('=' * 76)

def run_demonstration() -> None:
    # -------------------------------------------------------------
    # Test Program: Nested Lexical Scoping and Arithmetic
    # let x = 5 in
    #   let y = x * 2 in
    #     (x + y) * 3
    # -------------------------------------------------------------
    program = Flow.let(
        'x',
        Flow.lit(5),
        Flow.let(
            'y',
            Flow.bin('*', Flow.var('x'), Flow.lit(2)),
            Flow.bin(
                '*',
                Flow.bin('+', Flow.var('x'), Flow.var('y')),
                Flow.lit(3),
            ),
        ),
    )

    banner('1. Initial Program AST Structure')
    print(f"Expression: {format_ast(program)}")

    # -------------------------------------------------------------
    # Execution Model A: Dictionary Environment Model
    # -------------------------------------------------------------
    banner('2. Execution via Dictionary Notional Machine')
    dict_stepper = DictionaryStepper()
    dict_result = dict_stepper.evaluate(program)
    dict_trace = dict_stepper.get_trace()

    print(f"Final Evaluated Value: {dict_result}\n")
    print("State Transition Log:")
    for step in dict_trace:
        summary_str = step.expression_summary.ljust(28)
        snapshot_str = json.dumps(step.environment_snapshot)
        print(
            f"  [Step {step.step_number}] Action: {summary_str} | Env Snapshot: {snapshot_str}"
        )

    # -------------------------------------------------------------
    # Execution Model B: Pure Substitutional Term Rewriting
    # -------------------------------------------------------------
    banner('3. Execution via Substitutional Reduction Model')
    subst_stepper = SubstitutionStepper()
    subst_steps = subst_stepper.trace_reduction(program)

    print("Term Rewriting Trace:")
    for step in subst_steps:
        print(f"\n  Step {step.step_number}:")
        print(f"    Before: {step.before}")
        print(f"    Action: {step.action}")
        print(f"    After:  {step.after}")

    banner('4. Cross-Model Semantic Verification')
    final_subst_value = subst_steps[-1].after if subst_steps else None
    print(f"Dictionary Machine Result:     {dict_result}")
    print(f"Substitutional Machine Result: {final_subst_value}")
    print(f"Equivalence Confirmed:         {str(dict_result) == final_subst_value}")

if __name__ == '__main__':
    run_demonstration()