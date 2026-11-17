"""
Recursive mathematical operations comparing Linear O(N) vs Fast Divide-and-Conquer O(log N).
"""

from __future__ import annotations
from dataclasses import dataclass

@dataclass(frozen=True, slots=True)
class ExponentiationMetrics:
    result: int
    multiplication_steps: int

def power_linear(base: int, exp: int) -> ExponentiationMetrics:
    """
    Computes base^exp using Naive Linear Recursion: O(N) multiplication steps.
    """
    steps = 0

    def recurse(b: int, e: int) -> int:
        nonlocal steps
        if e == 0:
            return 1
        steps += 1
        return b * recurse(b, e - 1)

    res = recurse(base, exp)
    return ExponentiationMetrics(result=res, multiplication_steps=steps)

def power_fast(base: int, exp: int) -> ExponentiationMetrics:
    """
    Computes base^exp using Fast Divide-and-Conquer (Repeated Squaring): O(log N) steps.
    """
    steps = 0

    def recurse(b: int, e: int) -> int:
        nonlocal steps
        if e == 0:
            return 1
        if e == 1:
            return b

        if e % 2 == 0:
            steps += 1
            half = recurse(b, e // 2)
            return half * half
        else:
            steps += 2
            half = recurse(b, (e - 1) // 2)
            return b * half * half

    res = recurse(base, exp)
    return ExponentiationMetrics(result=res, multiplication_steps=steps)