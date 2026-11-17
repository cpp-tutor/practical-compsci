"""
Guarded conditional expression evaluator with short-circuit verification
and branch telemetry logging.
"""

from __future__ import annotations
from dataclasses import dataclass
from typing import Literal, Mapping, TypeAlias
from ast_nodes import Expr, PrimitiveValue, LiteralExpr, VariableExpr, BinaryExpr, LetExpr, IfExpr, StatementIfNode

Environment: TypeAlias = Mapping[str, PrimitiveValue]

@dataclass(frozen=True, slots=True)
class BranchEvaluationLog:
    branch_taken: Literal['then', 'else']
    discarded_branch: Expr

class GuardedEvaluator:
    """
    Evaluates expressions with lazy branch guards, ensuring discarded paths
    never trigger runtime faults (e.g. division by zero).
    """

    def __init__(self) -> None:
        self._branch_log: list[BranchEvaluationLog] = []

    def get_branch_history(self) -> tuple[BranchEvaluationLog, ...]:
        return tuple(self._branch_log)

    def evaluate(self, expr: Expr, env: Environment | None = None) -> PrimitiveValue:
        if env is None:
            env = {}

        match expr:
            case LiteralExpr(value):
                return value

            case VariableExpr(name):
                if name not in env:
                    raise NameError(f"Unbound variable '{name}'.")
                return env[name]

            case BinaryExpr(op, left, right):
                left_val = self.evaluate(left, env)
                right_val = self.evaluate(right, env)
                return self._eval_binary(op, left_val, right_val)

            case LetExpr(name, initializer, body):
                init_val = self.evaluate(initializer, env)
                new_env = {**env, name: init_val}
                return self.evaluate(body, new_env)

            case IfExpr(condition, then_branch, else_branch):
                # Evaluate the guard condition strictly
                cond_val = self.evaluate(condition, env)
                if not isinstance(cond_val, bool):
                    raise TypeError(
                        f"Type Error: Conditional guard must evaluate to Boolean, "
                        f"received '{type(cond_val).__name__}' ({cond_val})."
                    )

                if cond_val is True:
                    self._branch_log.append(
                        BranchEvaluationLog(
                            branch_taken='then',
                            discarded_branch=else_branch,
                        )
                    )
                    # Lazy evaluation: only evaluate then_branch
                    return self.evaluate(then_branch, env)
                else:
                    self._branch_log.append(
                        BranchEvaluationLog(
                            branch_taken='else',
                            discarded_branch=then_branch,
                        )
                    )
                    # Lazy evaluation: only evaluate else_branch
                    return self.evaluate(else_branch, env)

            case StatementIfNode():
                raise RuntimeError(
                    "StatementIfNode must be desugared into functional IfExpr prior to evaluation."
                )

    def _eval_binary(
        self,
        op: str,
        left: PrimitiveValue,
        right: PrimitiveValue,
    ) -> PrimitiveValue:
        # In Python, bool is a subclass of int; check bools first
        if isinstance(left, bool) and isinstance(right, bool):
            match op:
                case '&&': return left and right
                case '||': return left or right
                case '==': return left == right
                case '!=': return left != right

        elif (
            isinstance(left, (int, float))
            and isinstance(right, (int, float))
            and not isinstance(left, bool)
            and not isinstance(right, bool)
        ):
            match op:
                case '+': return left + right
                case '-': return left - right
                case '*': return left * right
                case '/':
                    if right == 0:
                        raise ZeroDivisionError("Division by zero.")
                    return (
                        left // right
                        if isinstance(left, int) and isinstance(right, int)
                        else left / right
                    )
                case '==': return left == right
                case '!=': return left != right
                case '<': return left < right
                case '<=': return left <= right
                case '>': return left > right
                case '>=': return left >= right

        if op == '==':
            return left == right
        if op == '!=':
            return left != right

        raise TypeError(
            f"Incompatible types for operator '{op}': "
            f"({type(left).__name__}, {type(right).__name__})."
        )