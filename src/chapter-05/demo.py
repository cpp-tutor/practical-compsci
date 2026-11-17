"""
Execution harness verifying lexical scope closures, multi-argument frames,
and higher-order function composition pipelines.
"""

from __future__ import annotations
from ast_nodes import Fn
from interpreter import FunctionInterpreter
from higher_order import pipe, curry

def banner(title: str) -> None:
    print('\n' + '=' * 76)
    print(f"  {title.upper()}")
    print('=' * 76)

def run_demonstrations() -> None:
    interpreter = FunctionInterpreter()

    # -------------------------------------------------------------
    # Test 1: Lexical Closure and Environment Retention
    # let makeAdder = (x) => ((y) => x + y) in
    #   let addFive = makeAdder(5) in
    #     addFive(10)
    # -------------------------------------------------------------
    banner('1. Lexical Scope Closure Verification (makeAdder)')

    closure_program = Fn.let(
        'makeAdder',
        Fn.lambda_(['x'], Fn.lambda_(['y'], Fn.bin('+', Fn.var('x'), Fn.var('y')))),
        Fn.let(
            'addFive',
            Fn.call(Fn.var('makeAdder'), Fn.lit(5)),
            Fn.call(Fn.var('addFive'), Fn.lit(10)),
        ),
    )

    result1 = interpreter.evaluate(closure_program)
    print('Program: let makeAdder = (x) => ((y) => x + y)...')
    print(f"Evaluated addFive(10): Result = {result1} (Expected: 15)")

    # -------------------------------------------------------------
    # Test 2: Multi-Argument Invocations and Stack Frame Tracking
    # let hypotenuseSquared = (a, b) => (a * a) + (b * b) in
    #   hypotenuseSquared(3, 4)
    # -------------------------------------------------------------
    banner('2. First-Class Multi-Argument Execution')

    math_program = Fn.let(
        'hypotenuseSquared',
        Fn.lambda_(
            ['a', 'b'],
            Fn.bin(
                '+',
                Fn.bin('*', Fn.var('a'), Fn.var('a')),
                Fn.bin('*', Fn.var('b'), Fn.var('b')),
            ),
        ),
        Fn.call(Fn.var('hypotenuseSquared'), Fn.lit(3), Fn.lit(4)),
    )

    result2 = interpreter.evaluate(math_program)
    print(f"Evaluated hypotenuseSquared(3, 4): Result = {result2} (Expected: 25)")
    print(f"Maximum Call Stack Depth Reached: {interpreter.get_max_stack_depth()}")

    # -------------------------------------------------------------
    # Test 3: Higher-Order Function Composition Pipeline
    # -------------------------------------------------------------
    banner('3. Pure Higher-Order Function Composition')

    add_ten = lambda n: n + 10
    square = lambda n: n * n
    format_result = lambda n: f"Resulting Value: [{n}]"

    # Pipeline: (x + 10) -> squared -> formatted
    pipeline = pipe(add_ten, pipe(square, format_result))
    print(f"Executed Pure Pipe on input 2: {pipeline(2)}")  # (2 + 10)^2 = 144

    curried_add = curry(lambda a, b: a + b)
    plus_twenty = curried_add(20)
    print(f"Curried Function plus_twenty(5): {plus_twenty(5)} (Expected: 25)")

if __name__ == '__main__':
    run_demonstrations()