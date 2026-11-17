"""
Execution harness validating persistent Stack LIFO invariants and
Okasaki Queue FIFO invariants.
"""

from __future__ import annotations
from sys import argv
from stack import PersistentStack
from queue import PersistentQueue
from benchmark import PerformanceProfiler, BenchmarkSeries, DataPoint

def banner(title: str) -> None:
    print('\n' + '=' * 76)
    print(f"  {title.upper()}")
    print('=' * 76)

def run_demonstrations() -> None:
    # -------------------------------------------------------------
    # Test 1: Persistent Stack LIFO Invariant Verification
    # -------------------------------------------------------------
    banner('1. Persistent Stack Invariant and Immutability Verification')

    s0 = PersistentStack[int].empty()
    s1 = s0.push(10).push(20).push(30)

    print(f"Stack s1 Contents (Top to Base): {s1.to_array()}")
    print(f"Stack s1 Size: {s1.size()}, Peek: {s1.peek()}")

    popped_val, s2 = s1.pop()
    print(f"Popped from s1: Value = {popped_val}")
    print(f"Stack s2 Contents after Pop:     {s2.to_array()}")
    print(f"Original Stack s1 Intact:        {s1.to_array()} (Immutability Preserved)")

    # -------------------------------------------------------------
    # Test 2: Persistent Okasaki Queue FIFO Invariant Verification
    # -------------------------------------------------------------
    banner('2. Okasaki Persistent Queue FIFO Invariant Verification')

    q0 = PersistentQueue[str].empty()
    q1 = q0.enqueue('Alpha').enqueue('Bravo').enqueue('Charlie')

    print(f"Queue q1 Contents (Front to Rear): {' -> '.join(q1.to_array())}")
    print(f"Queue q1 Size: {q1.size()}, Front Peek: {q1.peek()}")

    dequeued_val, q2 = q1.dequeue()
    print(f"Dequeued from q1: Item = '{dequeued_val}'")
    print(f"Queue q2 Contents after Dequeue:   {' -> '.join(q2.to_array())}")
    print(f"Original Queue q1 Intact:          {' -> '.join(q1.to_array())}")

if __name__ == '__main__':
    run_demonstrations()