"""
Inductive Singly-Linked List data structure backing persistent Stacks and Queues.
"""

from __future__ import annotations
from dataclasses import dataclass
from typing import Callable, Generic, Literal, Sequence, TypeAlias, TypeVar

T = TypeVar('T')

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
        current: FList[T] = NIL
        for item in reversed(arr):
            current = Cons(item, current)
        return current

    @staticmethod
    def to_array(lst: FList[T]) -> list[T]:
        result: list[T] = []
        curr = lst
        while isinstance(curr, Cons):
            result.append(curr.head)
            curr = curr.tail
        return result

    @staticmethod
    def reverse(lst: FList[T]) -> FList[T]:
        def rev_helper(remaining: FList[T], acc: FList[T]) -> FList[T]:
            match remaining:
                case Nil():
                    return acc
                case Cons(head, tail):
                    return rev_helper(tail, Cons(head, acc))

        return rev_helper(lst, NIL)