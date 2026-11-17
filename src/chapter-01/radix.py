"""
Arbitrary radix conversion engine, implementing the division-remainder
algorithm and positional evaluation across arbitrary radices.
"""

from bit import Radix
from typing import Final

DIGIT_ALPHABET: Final = '0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ'

def formatRadix(value: int, radix: Radix) -> str:
    """
    Converts a non-negative decimal integer into an arbitrary radix string.
    Implements the Division-Remainder algorithm.
    """
    if radix < 2 or radix > len(DIGIT_ALPHABET):
        raise ValueError(f'Radix must be between 2 and {len(DIGIT_ALPHABET)} (have {radix})')
    if value < 0:
        raise ValueError('Value must be zero or positive')
    if value == 0:
        return '0'
    
    current = value
    result = ''
    while (current > 0): # Division-Remainder algorithm
        remainder = current % radix # Modulo operator obtains remainder from division
        result = DIGIT_ALPHABET[remainder] + result
        current //= radix # Integer (truncating) division operator
    
    return result

def parseRadix(s: str, radix: Radix) -> int:
    """Parses an arbitrary radix string back into an integer."""
    if radix < 2 or radix > len(DIGIT_ALPHABET):
        raise ValueError(f'Radix must be between 2 and {len(DIGIT_ALPHABET)} (have {radix})')
    cleaned = s.strip().upper()
    if cleaned == '':
        raise ValueError('Cannot convert empty input string')
    accumulator = 0

    for index in range(len(cleaned)):
        accumulator *= radix # To accommodate next digit, multiply all existing digits by radix
        character = cleaned[index]
        digitValue = DIGIT_ALPHABET.find(character)
        if digitValue == -1:
            raise ValueError(f'Character {character} is not in {DIGIT_ALPHABET}')
        if digitValue >= radix:
            raise ValueError(f'Character {character} has too high a value ({digitValue}) for this radix ({radix})')
        accumulator += digitValue # Add the next digit value as the least significant digit

    return accumulator

def convertBase(s: str, fromRadix: Radix, toRadix: Radix) -> str:
    """Convert directly between two arbitrary radices."""
    numericalValue = parseRadix(s, fromRadix)
    return formatRadix(numericalValue, toRadix)

if __name__ == '__main__':
    assert(formatRadix(256, Radix.HEX) == '100')
    assert(parseRadix('400', Radix.HEX) == 1024)
    assert(convertBase('177', Radix.OCT, Radix.BIN) == '1111111')
