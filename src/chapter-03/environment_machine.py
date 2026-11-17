"""
Dictionary / Environment Notional Machine.
Executes programs sequentially by updating variable-to-value bindings across lexical frames.
"""

from __future__ import annotations
from dataclasses import dataclass
from typing import Mapping
from ast_nodes import Expr, PrimitiveValue, LiteralExpr, VariableExpr, BinaryExpr, LetExpr, AssignExpr, SequenceExpr, BlockExpr

class Environment:
    """
    An Environment Frame represents a scope dictionary with an optional parent pointer.
    """

    def __init__(self, parent: Environment | None = None) -> None:
        self._store: dict[str, PrimitiveValue] = {}
        self.parent: Environment | None = parent

    def define(self, name: str, value: PrimitiveValue) -> None:
        """Declares a new variable in the local scope frame."""
        self._store[name] = value

    def lookup(self, name: str) -> PrimitiveValue:
        """Resolves a variable by walking up the lexical environment chain."""
        if name in self._store:
            return self._store[name]
        if self.parent is not None:
            return self.parent.lookup(name)
        raise NameError(f"Runtime Error: Unbound identifier '{name}'.")

    def assign(self, name: str, value: PrimitiveValue) -> None:
        """Mutates an existing variable binding in the closest enclosing scope frame."""
        if name in self._store:
            self._store[name] = value
            return
        if self.parent is not None:
            self.parent.assign(name, value)
            return
        raise NameError(f"Runtime Error: Cannot assign to undeclared identifier '{name}'.")

    def snapshot(self) -> dict[str, PrimitiveValue]:
        """Produces a snapshot representation of all visible bindings."""
        parent_snapshot = self.parent.snapshot() if self.parent is not None else {}
        return {**parent_snapshot, **self._store}

@dataclass(frozen=True, slots=True)
class StateTraceStep:
    step_number: int
    expression_summary: str
    environment_snapshot: dict[str, PrimitiveValue]
    evaluated_value: PrimitiveValue

class DictionaryStepper:
    """
    Evaluates an AST using the Dictionary Environment Model, recording full state transitions.
    """

    def __init__(self) -> None:
        self._step_count: int = 0
        self._trace_log: list[StateTraceStep] = []

    def get_trace(self) -> tuple[StateTraceStep, ...]:
        return tuple(self._trace_log)

    def evaluate(self, expr: Expr, env: Environment | None = None) -> PrimitiveValue:
        if env is None:
            env = Environment()

        match expr:
            case LiteralExpr(value):
                return value

            case VariableExpr(name):
                val = env.lookup(name)
                self._record_step(f"Lookup '{name}'", env, val)
                return val

            case BinaryExpr(op, left, right):
                left_val = self.evaluate(left, env)
                right_val = self.evaluate(right, env)
                result = self._apply_binary_op(op, left_val, right_val)
                self._record_step(f"Eval Binary: ({left_val} {op} {right_val})", env, result)
                return result

            case LetExpr(name, initializer, body):
                init_val = self.evaluate(initializer, env)
                # Create a new child lexical environment frame for the body
                inner_env = Environment(parent=env)
                inner_env.define(name, init_val)
                self._record_step(f"Bind '{name}' = {init_val}", inner_env, init_val)
                return self.evaluate(body, inner_env)

            case AssignExpr(name, value):
                new_val = self.evaluate(value, env)
                env.assign(name, new_val)
                self._record_step(f"Mutate '{name}' = {new_val}", env, new_val)
                return new_val

            case SequenceExpr(expressions):
                last_value: PrimitiveValue = 0
                for sub_expr in expressions:
                    last_value = self.evaluate(sub_expr, env)
                return last_value

            case BlockExpr(body):
                block_env = Environment(parent=env)
                return self.evaluate(body, block_env)

    def _apply_binary_op(
        self,
        op: str,
        left: PrimitiveValue,
        right: PrimitiveValue,
    ) -> PrimitiveValue:
        # In Python, bool is a subclass of int, so we check bool explicitly first
        if isinstance(left, bool) and isinstance(right, bool):
            match op:
                case '&&': return left and right
                case '||': return left or right
                case '==': return left == right

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
                        raise ZeroDivisionError("Runtime Error: Division by zero.")
                    return (
                        left // right
                        if isinstance(left, int) and isinstance(right, int)
                        else left / right
                    )
                case '==': return left == right
                case '<': return left < right

        if op == '==':
            return left == right

        raise TypeError(
            f"Runtime Error: Illegal binary operation ({type(left).__name__} {op} {type(right).__name__})."
        )

    def _record_step(
        self,
        summary: str,
        env: Environment,
        val: PrimitiveValue,
    ) -> None:
        self._step_count += 1
        self._trace_log.append(
            StateTraceStep(
                step_number=self._step_count,
                expression_summary=summary,
                environment_snapshot=env.snapshot(),
                evaluated_value=val,
            )
        )