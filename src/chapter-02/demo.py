"""
Execution harness validating the implementation against the demonstrations in Chapter 2.
"""

from ast_nodes import Logic
from evaluator import format_expression
from truth_table import build_truth_table, print_truth_table, synthesise_sop
from prover import verify_equivalence, is_tautology
from circuit import Multiplexer2to1

def banner(title: str) -> None:
    print('\n' + '=' * 72)
    print(f"  {title.upper()}")
    print('=' * 72)

def run_demonstrations() -> None:
    # -------------------------------------------------------------
    # Test 1: Truth Table Generation and Canonical SOP Synthesis
    # -------------------------------------------------------------
    banner('1. Truth Table Generation and Canonical SOP Synthesis')

    # Define Majority Function: Maj(A, B, C) = (A ∧ B) ∨ (B ∧ C) ∨ (A ∧ C)
    a = Logic.var('A')
    b = Logic.var('B')
    c = Logic.var('C')

    majority_expr = Logic.or_(
        Logic.or_(Logic.and_(a, b), Logic.and_(b, c)),
        Logic.and_(a, c),
    )

    maj_table = build_truth_table(majority_expr)
    print_truth_table(maj_table)

    sop_synthesised = synthesise_sop(maj_table)
    print('\nCanonical SOP Expression:')
    print(f"  {format_expression(sop_synthesised)}")

    sop_equivalence = verify_equivalence(majority_expr, sop_synthesised)
    print(f"\nIs Synthesised SOP Equivalent to Original? {sop_equivalence.is_equivalent}")

    # -------------------------------------------------------------
    # Test 2: Formal Verification of De Morgan's Laws
    # -------------------------------------------------------------
    banner("2. Formal Proof of De Morgan's Laws via Exhaustive Search")

    x = Logic.var('X')
    y = Logic.var('Y')

    # Law 1: ¬(X ∧ Y) ≡ ¬X ∨ ¬Y
    de_morgan_1_left = Logic.not_(Logic.and_(x, y))
    de_morgan_1_right = Logic.or_(Logic.not_(x), Logic.not_(y))
    proof1 = verify_equivalence(de_morgan_1_left, de_morgan_1_right)

    print('Proof 1: ¬(X ∧ Y) ≡ (¬X ∨ ¬Y)')
    print(f"  Verified across {proof1.total_checked} states: Equivalence = {proof1.is_equivalent}")

    # Law 2: ¬(X ∨ Y) ≡ ¬X ∧ ¬Y
    de_morgan_2_left = Logic.not_(Logic.or_(x, y))
    de_morgan_2_right = Logic.and_(Logic.not_(x), Logic.not_(y))
    proof2 = verify_equivalence(de_morgan_2_left, de_morgan_2_right)

    print('\nProof 2: ¬(X ∨ Y) ≡ (¬X ∧ ¬Y)')
    print(f"  Verified across {proof2.total_checked} states: Equivalence = {proof2.is_equivalent}")

    # Law of Excluded Middle (Tautology): A ∨ ¬A
    excluded_middle = Logic.or_(a, Logic.not_(a))
    print(f"\nLaw of Excluded Middle (A ∨ ¬A) is Tautology: {is_tautology(excluded_middle)}")

    # -------------------------------------------------------------
    # Test 3: Structural Hardware Multiplexer Simulation
    # -------------------------------------------------------------
    banner('3. Hardware Gate Network Simulation (2-to-1 Multiplexer)')

    mux = Multiplexer2to1()

    # Test Case 1: Select = 0, Expect Output = Input0 (Set I0=1, I1=0)
    mux.input0.value = 1
    mux.input1.value = 0
    mux.select.value = 0
    print(f"MUX Config: I0=1, I1=0, Select=0 => Output = {mux.output.value} (Expected: 1)")

    # Test Case 2: Toggle Select to 1, Expect Output to switch to Input1 (0)
    mux.select.value = 1
    print(f"MUX Config: I0=1, I1=0, Select=1 => Output = {mux.output.value} (Expected: 0)")

    # Test Case 3: Set Input1 = 1 while Select = 1 => Expect Output = 1
    mux.input1.value = 1
    print(f"MUX Config: I0=1, I1=1, Select=1 => Output = {mux.output.value} (Expected: 1)")

if __name__ == '__main__':
    run_demonstrations()
