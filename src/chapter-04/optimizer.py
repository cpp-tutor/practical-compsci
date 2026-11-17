"""
Symbolic AST branch optimizer applying If-True, If-False, If-Irrelevant,
and contextual condition subsumption reduction rules.
"""

from __future__ import annotations
from dataclasses import dataclass
from ast_nodes import Expr, LiteralExpr, VariableExpr, BinaryExpr, LetExpr, IfExpr, StatementIfNode, Branch

def are_asts_equal(a: Expr, b: Expr) -> bool:
    """Checks structural equality between two AST expressions."""
    return a == b

@dataclass(frozen=True, slots=True)
class OptimizationMetrics:
    nodes_eliminated: int
    rules_applied: tuple[str, ...]

class BranchOptimizer:
    """
    Optimises an AST by recursively collapsing redundant branches and
    applying algebraic rules until a fixed point is reached.
    """

    def __init__(self) -> None:
        self._eliminated_count: int = 0
        self._applied_rules: list[str] = []

    def get_metrics(self) -> OptimizationMetrics:
        return OptimizationMetrics(
            nodes_eliminated=self._eliminated_count,
            rules_applied=tuple(self._applied_rules),
        )

    def optimize_to_fixed_point(self, expr: Expr) -> Expr:
        """Runs the optimiser to a fixed point (until no further reductions occur)."""
        current = expr
        iterations = 0
        max_iterations = 100

        while iterations < max_iterations:
            initial_count = self._eliminated_count
            current = self._optimize_node(current, frozenset())
            if self._eliminated_count == initial_count:
                break  # Fixed point reached
            iterations += 1

        return current

    def _optimize_node(
        self,
        expr: Expr,
        assumed_true_conditions: frozenset[Expr],
    ) -> Expr:
        match expr:
            case LiteralExpr(_) | VariableExpr(_):
                return expr

            case BinaryExpr(op, left, right):
                return Branch.bin(
                    op,
                    self._optimize_node(left, assumed_true_conditions),
                    self._optimize_node(right, assumed_true_conditions),
                )

            case LetExpr(name, initializer, body):
                return Branch.let(
                    name,
                    self._optimize_node(initializer, assumed_true_conditions),
                    self._optimize_node(body, assumed_true_conditions),
                )

            case IfExpr(condition, then_branch, else_branch):
                # Step 1: Recursively optimize the condition
                opt_cond = self._optimize_node(condition, assumed_true_conditions)

                # Check if condition is already known from enclosing context
                if opt_cond in assumed_true_conditions:
                    self._record_rule("Contextual Subsumption (Known True)", 2)
                    return self._optimize_node(then_branch, assumed_true_conditions)

                # Rule 1: If-True
                if isinstance(opt_cond, LiteralExpr) and opt_cond.value is True:
                    self._record_rule("Rule 1: If-True (Pruned Else-Branch)", 2)
                    return self._optimize_node(then_branch, assumed_true_conditions)

                # Rule 2: If-False
                if isinstance(opt_cond, LiteralExpr) and opt_cond.value is False:
                    self._record_rule("Rule 2: If-False (Pruned Then-Branch)", 2)
                    return self._optimize_node(else_branch, assumed_true_conditions)

                # Context extension for then-branch: condition is known True
                then_assumptions = assumed_true_conditions | {opt_cond}

                opt_then = self._optimize_node(then_branch, then_assumptions)
                opt_else = self._optimize_node(else_branch, assumed_true_conditions)

                # Rule 3: If-Irrelevant (Both branches compute identical subtrees)
                if are_asts_equal(opt_then, opt_else):
                    self._record_rule("Rule 3: If-Irrelevant (Identical Branches Collapsed)", 2)
                    return opt_then

                return Branch.if_(opt_cond, opt_then, opt_else)

            case StatementIfNode():
                return expr

    def _record_rule(self, name: str, nodes_removed: int) -> None:
        self._applied_rules.append(name)
        self._eliminated_count += nodes_removed