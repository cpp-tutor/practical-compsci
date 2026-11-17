"""
Polymorphic vector shape hierarchy demonstrating encapsulation,
subtyping, and the Composite design pattern.
"""

from __future__ import annotations
from abc import ABC, abstractmethod
from dataclasses import dataclass
from functools import reduce
import math
from typing import Sequence
from geometry import Point2D, BoundingBox

@dataclass(frozen=True, slots=True)
class StyleAttributes:
    fill_colour: str = '#38bdf8'
    stroke_colour: str = '#0f172a'
    stroke_width: float = 1.5
    opacity: float = 1.0

class Shape(ABC):
    """
    Abstract base class establishing the contract for all vector shapes.
    """

    def __init__(self, id_tag: str, style: StyleAttributes | None = None) -> None:
        self.id_tag: str = id_tag
        self._style: StyleAttributes = style if style is not None else StyleAttributes()

    @abstractmethod
    def compute_area(self) -> float:
        """Computes the enclosed area of the shape."""
        ...

    @abstractmethod
    def compute_perimeter(self) -> float:
        """Computes the total boundary perimeter of the shape."""
        ...

    @abstractmethod
    def get_bounding_box(self) -> BoundingBox:
        """Calculates the minimal enclosing Axis-Aligned Bounding Box."""
        ...

    @abstractmethod
    def render_svg(self) -> str:
        """Serialises the shape into an SVG vector XML element string."""
        ...

    @abstractmethod
    def translate(self, dx: float, dy: float) -> Shape:
        """Returns a new translated instance of the shape."""
        ...

    def _format_style_attributes(self) -> str:
        return (
            f'fill="{self._style.fill_colour}" '
            f'stroke="{self._style.stroke_colour}" '
            f'stroke-width="{self._style.stroke_width}" '
            f'opacity="{self._style.opacity}"'
        )

class Circle(Shape):
    """Concrete Shape: Circle."""

    def __init__(
        self,
        id_tag: str,
        centre: Point2D,
        radius: float,
        style: StyleAttributes | None = None,
    ) -> None:
        super().__init__(id_tag, style)
        if radius <= 0.0:
            raise ValueError(f"Circle radius must be strictly positive, received {radius}.")
        self._centre: Point2D = centre
        self._radius: float = radius

    def compute_area(self) -> float:
        return math.pi * self._radius * self._radius

    def compute_perimeter(self) -> float:
        return 2.0 * math.pi * self._radius

    def get_bounding_box(self) -> BoundingBox:
        return BoundingBox(
            min_x=self._centre.x - self._radius,
            min_y=self._centre.y - self._radius,
            max_x=self._centre.x + self._radius,
            max_y=self._centre.y + self._radius,
        )

    def render_svg(self) -> str:
        return (
            f'<circle id="{self.id_tag}" cx="{self._centre.x}" cy="{self._centre.y}" '
            f'r="{self._radius}" {self._format_style_attributes()} />'
        )

    def translate(self, dx: float, dy: float) -> Circle:
        return Circle(
            self.id_tag,
            self._centre.translate(dx, dy),
            self._radius,
            self._style,
        )

class Rectangle(Shape):
    """Concrete Shape: Rectangle."""

    def __init__(
        self,
        id_tag: str,
        origin: Point2D,
        width: float,
        height: float,
        style: StyleAttributes | None = None,
    ) -> None:
        super().__init__(id_tag, style)
        if width <= 0.0 or height <= 0.0:
            raise ValueError(
                f"Rectangle dimensions must be positive: width={width}, height={height}."
            )
        self._origin: Point2D = origin
        self._width: float = width
        self._height: float = height

    def compute_area(self) -> float:
        return self._width * self._height

    def compute_perimeter(self) -> float:
        return 2.0 * (self._width + self._height)

    def get_bounding_box(self) -> BoundingBox:
        return BoundingBox(
            min_x=self._origin.x,
            min_y=self._origin.y,
            max_x=self._origin.x + self._width,
            max_y=self._origin.y + self._height,
        )

    def render_svg(self) -> str:
        return (
            f'<rect id="{self.id_tag}" x="{self._origin.x}" y="{self._origin.y}" '
            f'width="{self._width}" height="{self._height}" rx="4" '
            f'{self._format_style_attributes()} />'
        )

    def translate(self, dx: float, dy: float) -> Rectangle:
        return Rectangle(
            self.id_tag,
            self._origin.translate(dx, dy),
            self._width,
            self._height,
            self._style,
        )

class CompoundShape(Shape):
    """
    Composite Pattern: CompoundShape treats a group of shapes polymorphically as a single shape.
    """

    def __init__(
        self,
        id_tag: str,
        children: Sequence[Shape],
        style: StyleAttributes | None = None,
    ) -> None:
        super().__init__(id_tag, style)
        if len(children) == 0:
            raise ValueError("CompoundShape must contain at least one child shape.")
        self._children: tuple[Shape, ...] = tuple(children)

    def compute_area(self) -> float:
        return sum(child.compute_area() for child in self._children)

    def compute_perimeter(self) -> float:
        return sum(child.compute_perimeter() for child in self._children)

    def get_bounding_box(self) -> BoundingBox:
        boxes = (child.get_bounding_box() for child in self._children)
        return reduce(lambda acc, b: acc.merge(b), boxes)

    def render_svg(self) -> str:
        rendered_children = "\n    ".join(child.render_svg() for child in self._children)
        return f'<g id="{self.id_tag}">\n    {rendered_children}\n  </g>'

    def translate(self, dx: float, dy: float) -> CompoundShape:
        translated_children = tuple(child.translate(dx, dy) for child in self._children)
        return CompoundShape(self.id_tag, translated_children, self._style)