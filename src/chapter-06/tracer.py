"""
Recursive Execution Call-Tree Tracer and ASCII Graph Formatter.
"""

from __future__ import annotations
from dataclasses import dataclass, field
import json
import time
from typing import Any, Callable, TypeVar

R = TypeVar('R')

@dataclass(slots=True)
class TreeNode:
    id: int
    function_name: str
    args: tuple[Any, ...]
    children: list[TreeNode] = field(default_factory=list)
    return_value: Any = None
    duration_ms: float = 0.0

@dataclass(frozen=True, slots=True)
class TreeMetrics:
    total_calls: int
    max_depth: int

class CallTreeTracer:
    """Execution Call-Tree Tracer for instrumenting recursive functions."""

    def __init__(self) -> None:
        self._node_sequence: int = 0
        self._stack: list[TreeNode] = []
        self.root: TreeNode | None = None

    def trace(self, fn_name: str, fn: Callable[..., R]) -> Callable[..., R]:
        """Wraps a recursive function to record its invocation graph."""

        def traced_wrapper(*args: Any) -> R:
            self._node_sequence += 1
            node = TreeNode(
                id=self._node_sequence,
                function_name=fn_name,
                args=args,
            )

            if not self._stack:
                self.root = node
            else:
                parent = self._stack[-1]
                parent.children.append(node)

            self._stack.append(node)
            start_time = time.perf_counter()

            try:
                result = fn(*args)
                node.return_value = result
                node.duration_ms = (time.perf_counter() - start_time) * 1000.0
                return result
            finally:
                self._stack.pop()

        return traced_wrapper

    def render_ascii_tree(
        self,
        node: TreeNode | None = None,
        prefix: str = '',
        is_last: bool = True,
    ) -> str:
        """Renders the recorded call tree into a formatted ASCII tree string."""
        if node is None:
            node = self.root

        if node is None:
            return '[Empty Call Tree]'

        branch = '└── ' if is_last else '├── '
        arg_str = ', '.join(json.dumps(a, default=str) for a in node.args)
        ret_str = json.dumps(node.return_value, default=str)
        line = f"{prefix}{branch}{node.function_name}({arg_str}) => {ret_str}\n"

        next_prefix = prefix + ('    ' if is_last else '│   ')
        children_output = ''

        for i, child in enumerate(node.children):
            is_child_last = i == len(node.children) - 1
            children_output += self.render_ascii_tree(
                child, next_prefix, is_child_last
            )

        return line + children_output

    def get_metrics(self, node: TreeNode | None = None) -> TreeMetrics:
        """Calculates structural metrics (total call count and maximum depth) of the tree."""
        if node is None:
            node = self.root

        if node is None:
            return TreeMetrics(total_calls=0, max_depth=0)

        total = 1
        max_child_depth = 0

        for child in node.children:
            child_metrics = self.get_metrics(child)
            total += child_metrics.total_calls
            if child_metrics.max_depth > max_child_depth:
                max_child_depth = child_metrics.max_depth

        return TreeMetrics(total_calls=total, max_depth=1 + max_child_depth)