"""
High-resolution performance benchmarking harness and dynamic SVG line-chart generator.
"""

from __future__ import annotations
from dataclasses import dataclass
import time
from typing import Callable, Sequence

@dataclass(frozen=True, slots=True)
class DataPoint:
    n: int
    duration_ms: float

@dataclass(frozen=True, slots=True)
class BenchmarkSeries:
    name: str
    color: str
    points: tuple[DataPoint, ...]

class PerformanceProfiler:
    """Micro-benchmarking runner using high-resolution performance timers."""

    @staticmethod
    def measure(task: Callable[[], None], warmup_runs: int = 3) -> float:
        """Runs warmup iterations before recording high-resolution elapsed time."""
        for _ in range(warmup_runs):
            task()

        start = time.perf_counter()
        task()
        return (time.perf_counter() - start) * 1000.0  # Duration in milliseconds

    @staticmethod
    def render_svg_chart(
        series_list: Sequence[BenchmarkSeries],
        width: int = 800,
        height: int = 420,
    ) -> str:
        """Generates a complete standalone SVG line chart comparing benchmark series."""
        padding = 60
        chart_width = width - padding * 2
        chart_height = height - padding * 2

        # Determine scale bounds
        max_n = 0
        max_time = 0.0

        for series in series_list:
            for p in series.points:
                if p.n > max_n:
                    max_n = p.n
                if p.duration_ms > max_time:
                    max_time = p.duration_ms

        if max_time == 0.0:
            max_time = 1.0

        def scale_x(n: int) -> float:
            return padding + (n / max_n) * chart_width

        def scale_y(t: float) -> float:
            return height - padding - (t / max_time) * chart_height

        paths_svg: list[str] = []
        legends_svg: list[str] = []

        for idx, series in enumerate(series_list):
            path_coords = [
                f"{'M' if i == 0 else 'L'} {scale_x(p.n):.1f} {scale_y(p.duration_ms):.1f}"
                for i, p in enumerate(series.points)
            ]
            path_data = " ".join(path_coords)
            paths_svg.append(
                f'<path d="{path_data}" fill="none" stroke="{series.color}" stroke-width="2.5" />'
            )

            # Draw data point circles
            for p in series.points:
                paths_svg.append(
                    f'<circle cx="{scale_x(p.n):.1f}" cy="{scale_y(p.duration_ms):.1f}" '
                    f'r="4" fill="{series.color}" />'
                )

            # Legend entry
            leg_y = padding + 20 + idx * 25
            legends_svg.append(f"""
    <rect x="{width - 240}" y="{leg_y - 10}" width="12" height="12" fill="{series.color}" rx="2" />
    <text x="{width - 220}" y="{leg_y}" fill="#e2e8f0" font-size="12">{series.name}</text>""")

        rendered_paths = "\n  ".join(paths_svg)
        rendered_legends = "".join(legends_svg)

        return f"""<svg viewBox="0 0 {width} {height}" width="100%" height="{height}" xmlns="http://www.w3.org/2000/svg" style="background-color: #0f172a; border-radius: 8px; font-family: 'Courier New', monospace;">
  <text x="{width // 2}" y="32" fill="#f8fafc" font-size="16" font-weight="bold" text-anchor="middle">Empirical Execution Scaling: Time vs Input Size (N)</text>
  
  <!-- Axes -->
  <line x1="{padding}" y1="{height - padding}" x2="{width - padding}" y2="{height - padding}" stroke="#475569" stroke-width="1.5" />
  <line x1="{padding}" y1="{padding}" x2="{padding}" y2="{height - padding}" stroke="#475569" stroke-width="1.5" />
  
  <!-- Labels -->
  <text x="{width // 2}" y="{height - 15}" fill="#94a3b8" font-size="12" text-anchor="middle">Input Size (N Items)</text>
  <text x="20" y="{height // 2}" fill="#94a3b8" font-size="12" text-anchor="middle" transform="rotate(-90 20 {height // 2})">Time (ms)</text>

  <!-- Data Paths -->
  {rendered_paths}

  <!-- Legends -->{rendered_legends}
</svg>"""