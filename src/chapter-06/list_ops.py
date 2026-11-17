"""
Inductive Singly-Linked List data structure and pure structural recursive operations.
"""

from __future__ import annotations
from dataclasses import dataclass
from typing import Callable, Generic, Literal, Sequence, TypeAlias, TypeVar

T = TypeVar('T')
A = TypeVar('A')
B = TypeVar('B')

@dataclass(frozen=True, slots=True)
class Nil:
    """The empty list (Base Case)."""
    kind: Literal['nil'] = 'nil'

NIL = Nil()

@dataclass(frozen=True, slots=True)
class Cons(Generic[T]):
    """An inductive cons-cell pairing a head element with a tail list."""
    head: T
    tail: FList[T]
    kind: Literal['cons'] = 'cons'

# Functional Inductive List Type: FList[T] = Nil | Cons[T]
FList: TypeAlias = Nil | Cons[T]

class LinkedList:
    """Smart constructors and pure structural recursion operators."""

    @staticmethod
    def nil() -> Nil:
        return NIL

    @staticmethod
    def cons(head: T, tail: FList[T]) -> FList[T]:
        return Cons(head, tail)

    @staticmethod
    def from_array(arr: Sequence[T]) -> FList[T]:
        """Constructs an immutable functional list from a native sequence."""
        current: FList[T] = NIL
        for item in reversed(arr):
            current = Cons(item, current)
        return current

    @staticmethod
    def to_array(lst: FList[T]) -> list[T]:
        """Converts an inductive list into a native Python list."""
        result: list[T] = []
        curr = lst
        while isinstance(curr, Cons):
            result.append(curr.head)
            curr = curr.tail
        return result

    @staticmethod
    def length(lst: FList[T]) -> int:
        """Structural Recursion: Computes the length of a list."""
        match lst:
            case Nil():
                return 0  # Base Case
            case Cons(_, tail):
                return 1 + LinkedList.length(tail)  # Inductive Step

    @staticmethod
    def append(list_a: FList[T], list_b: FList[T]) -> FList[T]:
        """Structural Recursion: Concatenates two lists (A ++ B)."""
        match list_a:
            case Nil():
                return list_b  # Base Case
            case Cons(head, tail):
                return Cons(head, LinkedList.append(tail, list_b))

    @staticmethod
    def map(lst: FList[A], fn: Callable[[A], B]) -> FList[B]:
        """Structural Recursion: Transforms each element via a mapping function."""
        match lst:
            case Nil():
                return NIL
            case Cons(head, tail):
                return Cons(fn(head), LinkedList.map(tail, fn))

    @staticmethod
    def filter(lst: FList[T], predicate: Callable[[T], bool]) -> FList[T]:
        """Structural Recursion: Filters elements satisfying a predicate."""
        match lst:
            case Nil():
                return NIL
            case Cons(head, tail):
                rest = LinkedList.filter(tail, predicate)
                return Cons(head, rest) if predicate(head) else rest

    @staticmethod
    def reverse(lst: FList[T]) -> FList[T]:
        """Tail-Recursive Reversal using an accumulator."""
        def rev_helper(remaining: FList[T], acc: FList[T]) -> FList[T]:
            match remaining:
                case Nil():
                    return acc
                case Cons(head, tail):
                    return rev_helper(tail, Cons(head, acc))

        return rev_helper(lst, NIL)

    @staticmethod
    def fold_right(lst: FList[A], zero: B, combine: Callable[[A, B], B]) -> B:
        """
        Right-Associative Fold (Catamorphism).
        fold_right([1, 2, 3], z, f) => f(1, f(2, f(3, z)))
        """
        match lst:
            case Nil():
                return zero
            case Cons(head, tail):
                return combine(head, LinkedList.fold_right(tail, zero, combine))

    @staticmethod
    def fold_left(lst: FList[A], acc: B, combine: Callable[[B, A], B]) -> B:
        """
        Left-Associative Fold with Accumulator (Tail-Recursive).
        fold_left([1, 2, 3], z, f) => f(f(f(z, 1), 2), 3)
        """
        match lst:
            case Nil():
                return acc
            case Cons(head, tail):
                return LinkedList.fold_left(tail, combine(acc, head), combine)