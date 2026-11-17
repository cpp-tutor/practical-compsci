"""
Substitutional / Term-Rewriting Execution Model.
Reduces expressions step-by-step through pure algebraic symbol substitution.
"""

from __future__ import annotations
from dataclasses import dataclass
from ast_nodes import Expr, PrimitiveValue, LiteralExpr, VariableExpr, BinaryExpr, LetExpr, AssignExpr, SequenceExpr, BlockExpr, Flow

def is_value(expr: Expr) -> bool:
    """Checks whether an AST node is an irreducible terminal literal value."""
    return isinstance(expr, LiteralExpr)

def format_ast(expr: Expr) -> str:
    """Pretty-prints an AST expression into a compact string representation."""
    match expr:
        case LiteralExpr(value):
            return f'"{value}"' if isinstance(value, str) else str(value)
        case VariableExpr(name):
            return name
        case BinaryExpr(op, left, right):
            return f"({format_ast(left)} {op} {format_ast(right)})"
        case LetExpr(name, initializer, body):
            return f"(let {name} = {format_ast(initializer)} in {format_ast(body)})"
        case AssignExpr(name, value):
            return f"({name} := {format_ast(value)})"
        case SequenceExpr(expressions):
            return f"({'; '.join(format_ast(e) for e in expressions)})"
        case BlockExpr(body):
            return f"{{ {format_ast(body)} }}"

def substitute(
    expr: Expr,
    target_var: str,
    replacement: PrimitiveValue,
) -> Expr:
    """
    Performs capture-avoiding syntactic substitution: expr[target_var ↦ replacement].
    Replaces every free occurrence of target_var within expr with replacement.
    """
    match expr:
        case LiteralExpr(_):
            return expr

        case VariableExpr(name):
            return Flow.lit(replacement) if name == target_var else expr

        case BinaryExpr(op, left, right):
            return Flow.bin(
                op,
                substitute(left, target_var, replacement),
                substitute(right, target_var, replacement),
            )

        case LetExpr(name, initializer, body):
            new_init = substitute(initializer, target_var, replacement)
            # If the inner let shadows target_var, do not substitute into the body
            if name == target_var:
                return Flow.let(name, new_init, body)
            return Flow.let(
                name,
                new_init,
                substitute(body, target_var, replacement),
            )

        case AssignExpr(name, value):
            return Flow.assign(name, substitute(value, target_var, replacement))

        case SequenceExpr(expressions):
            return Flow.seq(*(substitute(e, target_var, replacement) for e in expressions))

        case BlockExpr(body):
            return Flow.block(substitute(body, target_var, replacement))

@dataclass(frozen=True, slots=True)
class ReductionStep:
    step_number: int
    before: str
    action: str
    after: str

@dataclass(frozen=True, slots=True)
class StepOutcome:
    next_expr: Expr
    action: str

class SubstitutionStepper:
    """
    Evaluates an AST step-by-step using pure Term Rewriting (Small-Step Operational Semantics).
    """

    def reduce_step(self, expr: Expr) -> StepOutcome | None:
        """
        Executes a single atomic reduction step: e → e'.
        Returns None if the expression is already in normal form (terminal value).
        """
        if is_value(expr):
            return None

        match expr:
            case VariableExpr(name):
                raise NameError(
                    f"Substitutional Model Error: Free variable '{name}' encountered during reduction."
                )

            case BinaryExpr(op, left, right):
                # Left operand must be reduced to a value first
                if not is_value(left):
                    step = self.reduce_step(left)
                    if step is None:
                        raise RuntimeError("Failed to reduce left operand.")
                    return StepOutcome(
                        next_expr=Flow.bin(op, step.next_expr, right),
                        action=f"Reduce Left Operand: {step.action}",
                    )

                # Right operand reduced next
                if not is_value(right):
                    step = self.reduce_step(right)
                    if step is None:
                        raise RuntimeError("Failed to reduce right operand.")
                    return StepOutcome(
                        next_expr=Flow.bin(op, left, step.next_expr),
                        action=f"Reduce Right Operand: {step.action}",
                    )

                # Both sides are values: perform primitive reduction
                assert isinstance(left, LiteralExpr)
                assert isinstance(right, LiteralExpr)
                res = self._apply_primitive_op(op, left.value, right.value)
                return StepOutcome(
                    next_expr=Flow.lit(res),
                    action=f"Compute Primitive ({left.value} {op} {right.value}) → {res}",
                )

            case LetExpr(name, initializer, body):
                # Initializer must reduce to a value first
                if not is_value(initializer):
                    step = self.reduce_step(initializer)
                    if step is None:
                        raise RuntimeError("Failed to reduce let initializer.")
                    return StepOutcome(
                        next_expr=Flow.let(name, step.next_expr, body),
                        action=f"Reduce Initializer for '{name}': {step.action}",
                    )

                # Substitute the evaluated literal into the body
                assert isinstance(initializer, LiteralExpr)
                init_val = initializer.value
                substituted_body = substitute(body, name, init_val)
                return StepOutcome(
                    next_expr=substituted_body,
                    action=f"Substitute [{name} ↦ {init_val}] into body",
                )

            case BlockExpr(body):
                if is_value(body):
                    return StepOutcome(
                        next_expr=body,
                        action=f"Unwrap Block {{ {format_ast(body)} }} → {format_ast(body)}",
                    )
                step = self.reduce_step(body)
                if step is None:
                    raise RuntimeError("Failed to reduce block body.")
                return StepOutcome(
                    next_expr=Flow.block(step.next_expr),
                    action=f"Step inside Block: {step.action}",
                )

            case SequenceExpr(expressions):
                if len(expressions) == 0:
                    return StepOutcome(next_expr=Flow.lit(0), action="Empty sequence evaluates to 0")
                if len(expressions) == 1:
                    return StepOutcome(
                        next_expr=expressions[0],
                        action="Unwrap singleton sequence",
                    )
                first, *rest = expressions
                if is_value(first):
                    return StepOutcome(
                        next_expr=Flow.seq(*rest),
                        action=f"Discard evaluated value {format_ast(first)} in sequence",
                    )
                step = self.reduce_step(first)
                if step is None:
                    raise RuntimeError("Failed to reduce head of sequence.")
                return StepOutcome(
                    next_expr=Flow.seq(step.next_expr, *rest),
                    action=f"Step Head of Sequence: {step.action}",
                )

            case AssignExpr(_, _):
                raise NotImplementedError(
                    "Pure Substitutional Model does not support mutable assignments without state threading."
                )

    def trace_reduction(self, initial_expr: Expr) -> tuple[ReductionStep, ...]:
        """Reduces an expression to normal form, recording the step-by-step reduction trace."""
        steps: list[ReductionStep] = []
        current = initial_expr
        count = 0

        while not is_value(current):
            count += 1
            step = self.reduce_step(current)
            if step is None:
                break

            steps.append(
                ReductionStep(
                    step_number=count,
                    before=format_ast(current),
                    action=step.action,
                    after=format_ast(step.next_expr),
                )
            )
            current = step.next_expr

        return tuple(steps)

    def _apply_primitive_op(
        self,
        op: str,
        left: PrimitiveValue,
        right: PrimitiveValue,
    ) -> PrimitiveValue:
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
                        raise ZeroDivisionError("Division by zero in primitive evaluation.")
                    return (
                        left // right
                        if isinstance(left, int) and isinstance(right, int)
                        else left / right
                    )
                case '==': return left == right
                case '<': return left < right

        raise TypeError(f"Type error in primitive evaluation: {left} {op} {right}")