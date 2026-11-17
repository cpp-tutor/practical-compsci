"""
Vector canvas that aggregates heterogeneous shapes and compiles scenes into SVG documents.
"""

from __future__ import annotations
from functools import reduce
from shapes import Shape
from geometry import BoundingBox

class VectorCanvas:
    """Canvas managing a scene of polymorphically dispatched shapes."""

    def __init__(self, width: int, height: int, background_colour: str = '#0f172a') -> None:
        self.width: int = width
        self.height: int = height
        self.background_colour: str = background_colour
        self._shapes: list[Shape] = []

    def add_shape(self, shape: Shape) -> None:
        self._shapes.append(shape)

    def compute_total_area(self) -> float:
        return sum(s.compute_area() for s in self._shapes)

    def get_global_bounding_box(self) -> BoundingBox | None:
        if not self._shapes:
            return None
        boxes = (s.get_bounding_box() for s in self._shapes)
        return reduce(lambda acc, b: acc.merge(b), boxes)

    def render_to_svg_document(self) -> str:
        """
        Serialises all polymorphically registered shapes into a standalone SVG document string.
        """
        shape_elements = "\n".join(f"  {shape.render_svg()}" for shape in self._shapes)

        return (
            f'<svg viewBox="0 0 {self.width} {self.height}" width="100%" '
            f'height="{self.height}" xmlns="http://www.w3.org/2000/svg" '
            f'style="background-color: {self.background_colour}; border-radius: 8px;">\n'
            f'  <!-- Rendered Shapes via Polymorphic Dynamic Dispatch -->\n'
            f'{shape_elements}\n'
            f'</svg>'
        )