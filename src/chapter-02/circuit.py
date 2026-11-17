"""
Discrete event/signal simulation of physical hardware wires, logic gates, and composite multiplexers.
"""

from __future__ import annotations
from typing import Callable, Literal, TypeAlias

LogicLevel: TypeAlias = Literal[0, 1]

class Wire:
    """Represents a physical digital wire carrying a binary signal."""

    def __init__(self) -> None:
        self._signal: LogicLevel = 0
        self._listeners: list[Callable[[LogicLevel], None]] = []

    @property
    def value(self) -> LogicLevel:
        return self._signal

    @value.setter
    def value(self, new_level: LogicLevel) -> None:
        if self._signal != new_level:
            self._signal = new_level
            for listener in list(self._listeners):
                listener(new_level)

    def connect(self, listener: Callable[[LogicLevel], None]) -> None:
        self._listeners.append(listener)
        listener(self._signal)

class HardwareGates:
    """Fundamental hardware logic gate primitives."""

    @staticmethod
    def not_gate(input_wire: Wire, output_wire: Wire) -> None:
        def update(val: LogicLevel) -> None:
            output_wire.value = 1 if val == 0 else 0

        input_wire.connect(update)

    @staticmethod
    def and_gate(in_a: Wire, in_b: Wire, output_wire: Wire) -> None:
        def update(_: LogicLevel) -> None:
            output_wire.value = 1 if (in_a.value == 1 and in_b.value == 1) else 0

        in_a.connect(update)
        in_b.connect(update)

    @staticmethod
    def or_gate(in_a: Wire, in_b: Wire, output_wire: Wire) -> None:
        def update(_: LogicLevel) -> None:
            output_wire.value = 1 if (in_a.value == 1 or in_b.value == 1) else 0

        in_a.connect(update)
        in_b.connect(update)

    @staticmethod
    def nand_gate(in_a: Wire, in_b: Wire, output_wire: Wire) -> None:
        def update(_: LogicLevel) -> None:
            output_wire.value = 0 if (in_a.value == 1 and in_b.value == 1) else 1

        in_a.connect(update)
        in_b.connect(update)

    @staticmethod
    def xor_gate(in_a: Wire, in_b: Wire, output_wire: Wire) -> None:
        def update(_: LogicLevel) -> None:
            output_wire.value = 1 if in_a.value != in_b.value else 0

        in_a.connect(update)
        in_b.connect(update)

class Multiplexer2to1:
    """
    Structural 2-to-1 Multiplexer Circuit:
    Y = (I0 AND NOT S) OR (I1 AND S)
    """

    def __init__(self) -> None:
        self.input0 = Wire()
        self.input1 = Wire()
        self.select = Wire()
        self.output = Wire()

        not_select = Wire()
        and0_out = Wire()
        and1_out = Wire()

        HardwareGates.not_gate(self.select, not_select)
        HardwareGates.and_gate(self.input0, not_select, and0_out)
        HardwareGates.and_gate(self.input1, self.select, and1_out)
        HardwareGates.or_gate(and0_out, and1_out, self.output)
