#ifndef SIMPLIFIER_HPP
#define SIMPLIFIER_HPP

#include "SymbolicEngine.hpp"

namespace sym {

[[nodiscard]] inline Expression simplify(const Expression& expr) {
    return std::visit(overloaded{
        [](const Number& n) -> Expression {
            return num(n.value);
        },
        [](const Variable& v) -> Expression {
            return var(v.name);
        },
        [](const Negate& neg) -> Expression {
            Expression simplified_target = simplify(*neg.target);
            if (auto* n = std::get_if<Number>(&simplified_target.node)) {
                return num(-n->value);
            }
            return -simplified_target;
        },
        [](const Add& add) -> Expression {
            Expression l = simplify(*add.lhs);
            Expression r = simplify(*add.rhs);

            auto* num_l = std::get_if<Number>(&l.node);
            auto* num_r = std::get_if<Number>(&r.node);

            // Constant Folding: 2 + 3 -> 5
            if (num_l && num_r) return num(num_l->value + num_r->value);
            // Identity Rules: x + 0 -> x; 0 + x -> x
            if (num_l && num_l->value == 0.0) return r;
            if (num_r && num_r->value == 0.0) return l;

            return l + r;
        },
        [](const Sub& sub) -> Expression {
            Expression l = simplify(*sub.lhs);
            Expression r = simplify(*sub.rhs);

            auto* num_l = std::get_if<Number>(&l.node);
            auto* num_r = std::get_if<Number>(&r.node);

            // Constant Folding: 5 - 2 -> 3
            if (num_l && num_r) return num(num_l->value - num_r->value);
            // Identity Rule: x - 0 -> x
            if (num_r && num_r->value == 0.0) return l;

            return l - r;
        },
        [](const Mul& mul) -> Expression {
            Expression l = simplify(*mul.lhs);
            Expression r = simplify(*mul.rhs);

            auto* num_l = std::get_if<Number>(&l.node);
            auto* num_r = std::get_if<Number>(&r.node);

            // Constant Folding: 4 * 3 -> 12
            if (num_l && num_r) return num(num_l->value * num_r->value);
            // Zero Annihilation: x * 0 -> 0; 0 * x -> 0
            if ((num_l && num_l->value == 0.0) || (num_r && num_r->value == 0.0)) return num(0.0);
            // Multiplicative Identity: x * 1 -> x; 1 * x -> x
            if (num_l && num_l->value == 1.0) return r;
            if (num_r && num_r->value == 1.0) return l;

            return l * r;
        },
        [](const Div& div) -> Expression {
            Expression l = simplify(*div.lhs);
            Expression r = simplify(*div.rhs);

            auto* num_l = std::get_if<Number>(&l.node);
            auto* num_r = std::get_if<Number>(&r.node);

            if (num_r && num_r->value == 0.0) {
                throw std::runtime_error("Simplification Error: Static division by zero.");
            }
            if (num_l && num_r) return num(num_l->value / num_r->value);
            if (num_l && num_l->value == 0.0) return num(0.0);
            if (num_r && num_r->value == 1.0) return l;

            return l / r;
        },
        [](const Power& p) -> Expression {
            Expression b = simplify(*p.base);
            if (p.exponent == 0.0) return num(1.0);
            if (p.exponent == 1.0) return b;
            if (auto* num_b = std::get_if<Number>(&b.node)) {
                return num(std::pow(num_b->value, p.exponent));
            }
            return pow(b, p.exponent);
        }
    }, expr.node);
}

} // namespace sym

#endif // SIMPLIFIER_HPP