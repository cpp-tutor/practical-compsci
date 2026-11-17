"""
Evaluator implementing the Name-to-Body Rule and dynamic activation frame tracking.
"""

from __future__ import annotations
from dataclasses import dataclass
from ast_nodes import Expr, LiteralExpr, VariableExpr, BinaryExpr, LetExpr, IfExpr, LambdaExpr, CallExpr
from environment import Environment, RuntimeValue, Closure

@dataclass(frozen=True, slots=True)
class StackFrameInfo:
    frame_id: int
    function_name: str
    bound_arguments: dict[str, RuntimeValue]

class CallStack:
    """Manages the call stack of activation frames, detecting stack overflow."""

    def __init__(self, max_depth: int = 1024) -> None:
        self.max_depth: int = max_depth
        self._frames: list[StackFrameInfo] = []
        self._frame_sequence: int = 0

    def push(
        self, function_name: str, bound_arguments: dict[str, RuntimeValue]
    ) -> StackFrameInfo:
        if len(self._frames) >= self.max_depth:
            raise RecursionError(
                f"Call Stack Overflow: Maximum recursion depth of {self.max_depth} exceeded."
            )
        self._frame_sequence += 1
        frame = StackFrameInfo(
            frame_id=self._frame_sequence,
            function_name=function_name,
            bound_arguments=bound_arguments,
        )
        self._frames.append(frame)
        return frame

    def pop(self) -> None:
        self._frames.pop()

    @property
    def depth(self) -> int:
        return len(self._frames)

    def get_snapshot(self) -> tuple[StackFrameInfo, ...]:
        return tuple(self._frames)

class FunctionInterpreter:
    """
    Evaluator engine implementing static scoping, closure evaluation, and call tracing.
    """

    def __init__(self) -> None:
        self.call_stack: CallStack = CallStack()
        self._max_observed_stack_depth: int = 0

    def get_max_stack_depth(self) -> int:
        return self._max_observed_stack_depth

    def evaluate(self, expr: Expr, env: Environment | None = None) -> RuntimeValue:
        if env is None:
            env = Environment()

        if self.call_stack.depth > self._max_observed_stack_depth:
            self._max_observed_stack_depth = self.call_stack.depth

        match expr:
            case LiteralExpr(value):
                return value

            case VariableExpr(name):
                return env.lookup(name)

            case BinaryExpr(op, left, right):
                left_val = self.evaluate(left, env)
                right_val = self.evaluate(right, env)
                return self._eval_binary(op, left_val, right_val)

            case LetExpr(name, initializer, body):
                init_val = self.evaluate(initializer, env)
                # If binding a closure to an identifier, assign a diagnostic name tag
                if isinstance(init_val, Closure) and init_val.name_tag is None:
                    init_val.name_tag = name
                inner_env = Environment(parent=env)
                inner_env.define(name, init_val)
                return self.evaluate(body, inner_env)

            case IfExpr(condition, then_branch, else_branch):
                cond = self.evaluate(condition, env)
                if not isinstance(cond, bool):
                    raise TypeError(
                        f"Conditional guard must be Boolean, received '{type(cond).__name__}'."
                    )
                return (
                    self.evaluate(then_branch, env)
                    if cond
                    else self.evaluate(else_branch, env)
                )

            case LambdaExpr(parameters, body):
                # Closure creation: Captures current lexical environment by reference
                return Closure(parameters, body, env)

            case CallExpr(callee, arguments):
                # Step 1: Evaluate Callee Expression
                callee_val = self.evaluate(callee, env)
                if not isinstance(callee_val, Closure):
                    raise TypeError(
                        f"Runtime Error: Attempted to invoke non-function value '{callee_val}'."
                    )

                closure = callee_val

                # Step 2: Evaluate Actual Arguments in Current Caller Environment
                if len(arguments) != len(closure.parameters):
                    raise ValueError(
                        f"Arity Mismatch: Function expected {len(closure.parameters)} arguments, "
                        f"received {len(arguments)}."
                    )

                evaluated_args = [self.evaluate(arg, env) for arg in arguments]

                # Step 3 and 4: Instantiate Activation Frame linked to Captured Environment (Name-to-Body Rule)
                activation_frame = Environment(parent=closure.captured_environment)
                arg_record: dict[str, RuntimeValue] = {}

                for param_name, arg_val in zip(closure.parameters, evaluated_args):
                    activation_frame.define(param_name, arg_val)
                    arg_record[param_name] = arg_val

                # Push frame onto Call Stack for monitoring
                func_name = closure.name_tag or 'anonymous'
                self.call_stack.push(func_name, arg_record)

                # Step 5: Evaluate Function Body under Activation Frame
                try:
                    return self.evaluate(closure.body, activation_frame)
                finally:
                    self.call_stack.pop()

    def _eval_binary(
        self,
        op: str,
        left: RuntimeValue,
        right: RuntimeValue,
    ) -> RuntimeValue:
        if isinstance(left, bool) and isinstance(right, bool):
            match op:
                case '&&': return left and right
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
                        raise ZeroDivisionError("Division by zero.")
                    return (
                        left // right
                        if isinstance(left, int) and isinstance(right, int)
                        else left / right
                    )
                case '==': return left == right
                case '<': return left < right

        raise TypeError(
            f"Illegal binary operation on ({type(left).__name__} {op} {type(right).__name__})."
        )