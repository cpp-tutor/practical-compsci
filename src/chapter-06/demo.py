"""
Execution harness validating linked list structural recursion,
divide-and-conquer exponentiation, and recursive call-tree tracing.
"""

from __future__ import annotations
from list_ops import LinkedList
from math_ops import power_linear, power_fast
from tracer import CallTreeTracer

def banner(title: str) -> None:
    print('\n' + '=' * 76)
    print(f"  {title.upper()}")
    print('=' * 76)

def run_demonstrations() -> None:
    # -------------------------------------------------------------
    # Test 1: Structural Recursion over Singly-Linked Lists
    # -------------------------------------------------------------
    banner('1. Structural Induction on Linked Lists')

    raw_array = [10, 20, 30, 40, 50]
    lst = LinkedList.from_array(raw_array)

    print(f"Input List:      [{' -> '.join(map(str, LinkedList.to_array(lst)))}]")
    print(f"Length:          {LinkedList.length(lst)}")

    doubled = LinkedList.map(lst, lambda x: x * 2)
    print(f"Mapped (* 2):    [{' -> '.join(map(str, LinkedList.to_array(doubled)))}]")

    filtered = LinkedList.filter(lst, lambda x: x > 25)
    print(f"Filtered (> 25): [{' -> '.join(map(str, LinkedList.to_array(filtered)))}]")

    reversed_list = LinkedList.reverse(lst)
    print(f"Reversed:        [{' -> '.join(map(str, LinkedList.to_array(reversed_list)))}]")

    total_sum = LinkedList.fold_left(lst, 0, lambda acc, x: acc + x)
    print(f"Fold Sum:        {total_sum} (Expected: 150)")

    # -------------------------------------------------------------
    # Test 2: Divide-and-Conquer Complexity Comparison
    # -------------------------------------------------------------
    banner('2. Exponentiation Complexity: O(N) vs O(log N)')

    base = 2
    exponent = 64

    linear_run = power_linear(base, exponent)
    fast_run = power_fast(base, exponent)

    print(f"Calculating {base}^{exponent}:")
    print(f"  Linear Recursion Multiplications: {linear_run.multiplication_steps} steps")
    print(f"  Fast Squaring Multiplications:    {fast_run.multiplication_steps} steps")
    speedup = linear_run.multiplication_steps / fast_run.multiplication_steps
    print(f"  Algorithmic Speedup:              {speedup:.2f}x fewer operations")

    # -------------------------------------------------------------
    # Test 3: Tracing Call-Tree Graphs
    # -------------------------------------------------------------
    banner('3. Call-Tree Graph Visualisation (Fibonacci)')

    tracer = CallTreeTracer()

    # Define traced Fibonacci function
    traced_fib = None

    def fib(n: int) -> int:
        if n == 0:
            return 0
        if n == 1:
            return 1
        return traced_fib(n - 1) + traced_fib(n - 2)

    traced_fib = tracer.trace('fib', fib)

    fib_result = traced_fib(4)
    print(f"Computed fib(4) = {fib_result}\n")
    print('Execution Call Tree:')
    print(tracer.render_ascii_tree())

    metrics = tracer.get_metrics()
    print(f"Tree Metrics: Total Calls = {metrics.total_calls}, Max Depth = {metrics.max_depth}")

if __name__ == '__main__':
    run_demonstrations()