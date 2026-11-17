"""
Immutable 2D Cartesian geometry primitives and Axis-Aligned Bounding Boxes (AABB).
"""

from __future__ import annotations
from dataclasses import dataclass
import math

@dataclass(frozen=True, slots=True)
class Point2D:
    """Immutable 2D Cartesian Coordinate."""
    x: float
    y: float

    def translate(self, dx: float, dy: float) -> Point2D:
        return Point2D(self.x + dx, self.y + dy)

    def scale(self, factor: float, origin: Point2D | None = None) -> Point2D:
        if origin is None:
            origin = Point2D(0.0, 0.0)
        return Point2D(
            origin.x + (self.x - origin.x) * factor,
            origin.y + (self.y - origin.y) * factor,
        )

    def distance_to(self, other: Point2D) -> float:
        dx = self.x - other.x
        dy = self.y - other.y
        return math.hypot(dx, dy)

@dataclass(frozen=True, slots=True)
class BoundingBox:
    """Axis-Aligned Bounding Box (AABB) for spatial geometry."""
    min_x: float
    min_y: float
    max_x: float
    max_y: float

    def __post_init__(self) -> None:
        if self.min_x > self.max_x or self.min_y > self.max_y:
            raise ValueError(
                f"Invalid BoundingBox dimensions: [{self.min_x}, {self.min_y}] "
                f"to [{self.max_x}, {self.max_y}]"
            )

    @property
    def width(self) -> float:
        return self.max_x - self.min_x

    @property
    def height(self) -> float:
        return self.max_y - self.min_y

    def merge(self, other: BoundingBox) -> BoundingBox:
        return BoundingBox(
            min_x=min(self.min_x, other.min_x),
            min_y=min(self.min_y, other.min_y),
            max_x=max(self.max_x, other.max_x),
            max_y=max(self.max_y, other.max_y),
        )