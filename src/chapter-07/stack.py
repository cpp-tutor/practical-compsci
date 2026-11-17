"""
Persistent, purely functional Stack ADT backed by an inductive cons-list.
"""

from __future__ import annotations
from typing import Generic, Protocol, TypeVar
from list_ops import FList, Cons, Nil, LinkedList, NIL

T = TypeVar('T')

class PersistentStack(Generic[T]):
    """
    Purely functional persistent Stack backed by an inductive singly-linked list.
    All operations provide strict O(1) time and space guarantees via structural sharing.
    """

    def __init__(self, elements: FList[T], count: int) -> None:
        self._elements: FList[T] = elements
        self._count: int = count

    @classmethod
    def empty(cls) -> PersistentStack[T]:
        return cls(NIL, 0)

    def push(self, value: T) -> PersistentStack[T]:
        """O(1) time: Allocate a single cons-cell pointing to the existing list."""
        return PersistentStack(
            LinkedList.cons(value, self._elements),
            self._count + 1,
        )

    def pop(self) -> tuple[T, PersistentStack[T]]:
        """O(1) time: Return head value and tail list reference."""
        match self._elements:
            case Nil():
                raise IndexError("NoSuchElement: Cannot pop from an empty Stack.")
            case Cons(head, tail):
                remaining_stack = PersistentStack(tail, self._count - 1)
                return head, remaining_stack

    def peek(self) -> T:
        match self._elements:
            case Nil():
                raise IndexError("NoSuchElement: Cannot peek into an empty Stack.")
            case Cons(head, _):
                return head

    def is_empty(self) -> bool:
        return self._count == 0

    def size(self) -> int:
        return self._count

    def to_array(self) -> list[T]:
        return LinkedList.to_array(self._elements)