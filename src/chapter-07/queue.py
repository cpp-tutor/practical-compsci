"""
Purely functional persistent Queue implementing Chris Okasaki's two-list algorithm.
"""

from __future__ import annotations
from typing import Generic, Protocol, TypeVar
from list_ops import FList, Cons, Nil, LinkedList, NIL

T = TypeVar('T')

class PersistentQueue(Generic[T]):
    """
    Purely functional persistent Queue implementing Chris Okasaki's 2-list algorithm.
    Provides amortised O(1) enqueue and dequeue operations.
    """

    def __init__(self, front: FList[T], rear: FList[T], count: int) -> None:
        self._front: FList[T] = front
        self._rear: FList[T] = rear
        self._count: int = count

    @classmethod
    def empty(cls) -> PersistentQueue[T]:
        return cls(NIL, NIL, 0)

    def enqueue(self, value: T) -> PersistentQueue[T]:
        """
        Enqueues an element onto the rear list.
        O(1) worst-case time complexity.
        """
        return self._make_queue(
            self._front,
            LinkedList.cons(value, self._rear),
            self._count + 1,
        )

    def dequeue(self) -> tuple[T, PersistentQueue[T]]:
        """
        Dequeues an element from the front list.
        O(1) amortised time complexity.
        """
        match self._front:
            case Nil():
                raise IndexError("NoSuchElement: Cannot dequeue from an empty Queue.")
            case Cons(head, tail):
                next_queue = self._make_queue(
                    tail,
                    self._rear,
                    self._count - 1,
                )
                return head, next_queue

    def peek(self) -> T:
        match self._front:
            case Nil():
                raise IndexError("NoSuchElement: Cannot peek into an empty Queue.")
            case Cons(head, _):
                return head

    def is_empty(self) -> bool:
        return self._count == 0

    def size(self) -> int:
        return self._count

    def to_array(self) -> list[T]:
        front_array = LinkedList.to_array(self._front)
        rear_array = LinkedList.to_array(LinkedList.reverse(self._rear))
        return front_array + rear_array

    @classmethod
    def _make_queue(
        cls,
        front: FList[T],
        rear: FList[T],
        count: int,
    ) -> PersistentQueue[T]:
        """
        Enforces the Okasaki invariant: Front is only Nil if Rear is also Nil.
        If Front is Nil and Rear is non-empty, rotate:
            Front' = reverse(Rear), Rear' = Nil
        """
        if isinstance(front, Nil):
            return cls(LinkedList.reverse(rear), NIL, count)
        return cls(front, rear, count)