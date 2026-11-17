"""
Higher-Order Function (HOF) utilities: composition, piping, currying, and memoisation.
"""

from __future__ import annotations
import json
from typing import Callable, TypeVar

A = TypeVar('A')
B = TypeVar('B')
C = TypeVar('C')

UnaryFn = Callable[[A], B]

def compose(f: Callable[[B], C], g: Callable[[A], B]) -> Callable[[A], C]:
    """
    Pure Algebraic Function Composition: (f ∘ g)(x) = f(g(x))
    """
    return lambda x: f(g(x))

def pipe(f: Callable[[A], B], g: Callable[[B], C]) -> Callable[[A], C]:
    """
    Left-to-right pipeline: pipe(f, g)(x) = g(f(x))
    """
    return lambda x: g(f(x))

def curry(fn: Callable[[A, B], C]) -> Callable[[A], Callable[[B], C]]:
    """
    Currying transformation for binary functions: f(a, b) => f(a)(b)
    """
    return lambda a: lambda b: fn(a, b)

def memoize(fn: Callable[[A], B]) -> Callable[[A], B]:
    """
    Pure Memoisation Wrapper for deterministic unary functions.
    """
    cache: dict[str, B] = {}

    def memoized_wrapper(arg: A) -> B:
        key = json.dumps(arg, default=str)
        if key in cache:
            return cache[key]
        computed = fn(arg)
        cache[key] = computed
        return computed

    return memoized_wrapper