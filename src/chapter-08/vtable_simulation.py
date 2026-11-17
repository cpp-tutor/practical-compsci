"""
Explicit simulation of Virtual Method Tables (vtables) and vptr dynamic dispatch.
"""

from __future__ import annotations
from dataclasses import dataclass
import math
from typing import Any, Callable, TypeAlias

AreaFn: TypeAlias = Callable[['RawObjectInstance'], float]
RenderFn: TypeAlias = Callable[['RawObjectInstance'], str]

@dataclass(frozen=True, slots=True)
class VTable:
    """The Virtual Method Table (VTable): A static table of function pointers."""
    compute_area: AreaFn
    render_svg: RenderFn

@dataclass(slots=True)
class RawObjectInstance:
    """
    Raw Object Instance: Models a low-level C++ memory block with a hidden vptr at offset 0.
    """
    vptr: VTable
    fields: dict[str, Any]

# Static VTable Definition for Circle
def _circle_compute_area(self: RawObjectInstance) -> float:
    r: float = self.fields['radius']
    return math.pi * r * r

def _circle_render_svg(self: RawObjectInstance) -> str:
    x = self.fields['x']
    y = self.fields['y']
    r = self.fields['radius']
    return f'<circle cx="{x}" cy="{y}" r="{r}" fill="#38bdf8" />'

CircleVTable = VTable(
    compute_area=_circle_compute_area,
    render_svg=_circle_render_svg,
)

# Static VTable Definition for Rectangle
def _rectangle_compute_area(self: RawObjectInstance) -> float:
    w: float = self.fields['w']
    h: float = self.fields['h']
    return w * h

def _rectangle_render_svg(self: RawObjectInstance) -> str:
    x = self.fields['x']
    y = self.fields['y']
    w = self.fields['w']
    h = self.fields['h']
    return f'<rect x="{x}" y="{y}" width="{w}" height="{h}" fill="#4ade80" />'

RectangleVTable = VTable(
    compute_area=_rectangle_compute_area,
    render_svg=_rectangle_render_svg,
)

# Factory functions allocating raw instances with attached vptrs
def create_raw_circle(x: float, y: float, radius: float) -> RawObjectInstance:
    return RawObjectInstance(
        vptr=CircleVTable,
        fields={'x': x, 'y': y, 'radius': radius},
    )

def create_raw_rectangle(x: float, y: float, w: float, h: float) -> RawObjectInstance:
    return RawObjectInstance(
        vptr=RectangleVTable,
        fields={'x': x, 'y': y, 'w': w, 'h': h},
    )

# Simulated Dynamic Dispatch: Emulates indirect jump through vptr table slots
def dispatch_compute_area(obj: RawObjectInstance) -> float:
    return obj.vptr.compute_area(obj)  # Dereference vptr -> index compute_area -> execute

def dispatch_render_svg(obj: RawObjectInstance) -> str:
    return obj.vptr.render_svg(obj)  # Dereference vptr -> index render_svg -> execute