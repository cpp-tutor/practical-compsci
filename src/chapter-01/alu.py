"""
Emulation of physical silicon logic gates and assembly of the full Arithmetic Logic Unit.
"""

from bit import Bit, BitVector, ArithmeticResult, ALUFlags

class Gates:
    """Primitive Logic Gates using the Bit class: NOT, AND, OR and XOR."""
    @staticmethod
    def NOT(a: Bit) -> Bit:
        return Bit.TRUE if a == Bit.FALSE else Bit.FALSE

    @staticmethod
    def AND(a: Bit, b: Bit) -> Bit:
        return Bit.TRUE if a == Bit.TRUE and b == Bit.TRUE else Bit.FALSE

    @staticmethod
    def OR(a: Bit, b: Bit) -> Bit:
        return Bit.TRUE if a == Bit.TRUE or b == Bit.TRUE else Bit.FALSE

    @staticmethod
    def XOR(a: Bit, b: Bit) -> Bit:
        return Bit.TRUE if a != b else Bit.FALSE

def halfAdder(a: Bit, b: Bit):
    """Structural 1-bit Half Adder."""
    s = Gates.XOR(a, b)
    c = Gates.AND(a, b)
    return { 'sum': s, 'carry': c }

def fullAdder(a: Bit, b: Bit, cin: Bit):
    """Structural 1-bit Full Adder."""
    ha1 = halfAdder(a, b);
    ha2 = halfAdder(ha1['sum'], cin);
    cout = Gates.OR(ha1['carry'], ha2['carry']);
    return { 'sum': ha2['sum'], 'cout': cout }

def rippleCarryAdd(operandA: BitVector, operandB: BitVector, **kwargs) -> ArithmeticResult:
    """Generic fixed-width Ripple Carry Adder with full ALU Flag generation."""
    verbose = 'verbose' in kwargs and kwargs['verbose'] == True
    carryIn = Bit.TRUE if 'carryIn' in kwargs and kwargs['carryIn'] == True else Bit.FALSE
    if operandA.width != operandB.width:
        raise ValueError(f'Operands must be the same width (got {operandA.width} and {operandB.width})')
    if verbose:
        print(f'Adding A={str(operandA)} and B={str(operandB)} (Cin = {carryIn}):')

    width = operandA.width
    sumBits = [Bit.FALSE for index in range(width)]
    currentCarry = carryIn
    carryIntoMsb = carryOutOfMsb = Bit.FALSE

    for index in range(width):
        bitA = operandA[index]
        bitB = operandB[index]
        if index == width - 1:
            carryIntoMsb = currentCarry
        if verbose:
            print(f'Bit {index}: Cin = {currentCarry}, A = {bitA}, B = {bitB} (into Full Adder)')
        fa = fullAdder(bitA, bitB, currentCarry)
        if verbose:
            print(f"\tResult = {fa['sum']}, Cout = {fa['cout']} (out of Full Adder)")
        sumBits[width - index - 1] = fa['sum'] # [MSB, ..., LSB]
        currentCarry = fa['cout']
        if index == width - 1:
            carryOutOfMsb = currentCarry
            if verbose:
                print(f'\tCin({index}) = {carryIntoMsb}, Cout({index}) = {carryOutOfMsb} (Difference indicates overflow)')

    isNegative = sumBits[0] == Bit.TRUE
    isZero = not (Bit.TRUE in sumBits)
    isCarry = carryOutOfMsb == Bit.TRUE
    isOverflow = Gates.XOR(carryIntoMsb, carryOutOfMsb) == Bit.TRUE
    flags = ALUFlags(negative=isNegative, zero=isZero, carry=isCarry, overflow=isOverflow)
    if verbose:
        print(f'\nResult: {BitVector(sumBits).asSignedInteger()}, ALU Flags: {flags}')
    return ArithmeticResult(bits=BitVector(sumBits), flags=flags)
