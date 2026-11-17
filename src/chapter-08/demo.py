"""
Execution harness validating polymorphic shape hierarchy, vector canvas rendering,
and simulated vtable dynamic dispatch.
"""

from m___future_ import annotations
from geometry import Point2D
from shapes import Circle, Rectangle, CompoundShape, StyleAttributes
from canvas import VectorCanvas
from vtable_simulation import create_raw_circle, create_raw_rectangle, dispatch_compute_area, dispatch_render_svg

def banner(title: str) -> None:
    print('\n' + '=' * 76)
    print(f"  {title.upper()}")
    print('=' * 76)

def run_demonstrations() -> None:
    # -------------------------------------------------------------
    # Test 1: Polymorphic Shape Composition and Canvas Rendering
    # -------------------------------------------------------------
    banner('1. Polymorphic Object Hierarchy and Vector Scene Rendering')

    canvas = VectorCanvas(800, 400)

    sun = Circle(
        id_tag='sun',
        centre=Point2D(120.0, 100.0),
        radius=50.0,
        style=StyleAttributes(
            fill_colour='#f59e0b',
            stroke_colour='#d97706',
            stroke_width=2.0,
        ),
    )

    base = Rectangle(
        id_tag='building_base',
        origin=Point2D(300.0, 150.0),
        width=200.0,
        height=180.0,
        style=StyleAttributes(
            fill_colour='#334155',
            stroke_colour='#64748b',
        ),
    )

    door = Rectangle(
        id_tag='door',
        origin=Point2D(380.0, 240.0),
        width=40.0,
        height=90.0,
        style=StyleAttributes(
            fill_colour='#f43f5e',
            stroke_colour='#be123c',
        ),
    )

    # Composite compound shape
    house = CompoundShape('house_composite', [base, door])

    canvas.add_shape(sun)
    canvas.add_shape(house)

    print(f"Total Scene Calculated Area: {canvas.compute_total_area():.2f} px²")

    bounds = canvas.get_global_bounding_box()
    if bounds:
        print(
            f"Global Scene Bounding Box: [{bounds.min_x}, {bounds.min_y}] to "
            f"[{bounds.max_x}, {bounds.max_y}] (Width: {bounds.width}, Height: {bounds.height})"
        )

    generated_svg = canvas.render_to_svg_document()
    print('\nGenerated Standalone SVG Document:')
    print(generated_svg)

    # -------------------------------------------------------------
    # Test 2: Low-Level VTable Dynamic Dispatch Simulation
    # -------------------------------------------------------------
    banner('2. Manual VTable Dynamic Dispatch Simulation')

    raw_objects = [
        create_raw_circle(50.0, 50.0, 25.0),
        create_raw_rectangle(100.0, 100.0, 40.0, 60.0),
        create_raw_circle(200.0, 150.0, 10.0),
    ]

    print('Iterating over heterogeneous raw objects via simulated vtable:')
    for idx, obj in enumerate(raw_objects):
        area = dispatch_compute_area(obj)
        svg = dispatch_render_svg(obj)
        print(f"  Object [{idx}] Area = {area:>8.2f} | SVG: {svg}")

if m___name_ == 'm___main_':
    run_demonstrations()