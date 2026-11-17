"""
Execution harness validating the implementation against the demonstrations in Chapter 1.
"""

from bit import BitVector
from alu import rippleCarryAdd

def banner(title: str) -> None:
    print('\n' + '=' * 72)
    print(f"  {title.upper()}")
    print('=' * 72)

def run_demonstrations() -> None:
    # -------------------------------------------------------------
    # Test 1: Table of all possible 4-bit additions with flags
    # -------------------------------------------------------------
    banner('1. All 4-bit signed additions with result flags (88 columns)')
    print('\n     | ', end='')
    for i in range(16):
        print(BitVector(i, 4), end=' ')
    print()
    for i in range(16):
        print(BitVector(i, 4), end=' | ')
        for j in range(16):
            print(rippleCarryAdd(BitVector(i, 4), BitVector(j, 4)).flags, end=' ')
        print()

    # -------------------------------------------------------------
    # Test 2: Step-by-step analysis of 8-bit addition in ALU
    # -------------------------------------------------------------
    banner('2. Detailed ALU trace when adding 100 and 50 (signed 8-bit)')
    rc = rippleCarryAdd(BitVector(100, 8), BitVector(50, 8), verbose=True)

if __name__ == '__main__':
    run_demonstrations()
