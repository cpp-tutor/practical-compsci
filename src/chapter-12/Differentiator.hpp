#ifndef DIFFERENTIATOR_HPP
#define DIFFERENTIATOR_HPP

#include "SymbolicEngine.hpp"
#include "Simplifier.hpp"

namespace sym {

[[nodiscard]] inline Expression differentiate(const Expression& expr, const std::string& var_name) {
    return std::visit(overloaded{
        [](const Number&) -> Expression {
            return num(0.0); // d/dx (c) = 0
        },
        [&var_name](const Variable& v) -> Expression {
            return (v.name == var_name) ? num(1.0) : num(0.0); // d/dx (x) = 1, d/dx (y) = 0
        },
        [&var_name](const Negate& neg) -> Expression {
            return -differentiate(*neg.target, var_name);
        },
        [&var_name](const Add& add) -> Expression {
            // (u + v)' = u' + v'
            return differentiate(*add.lhs, var_name) + differentiate(*add.rhs, var_name);
        },
        [&var_name](const Sub& sub) -> Expression {
            // (u - v)' = u' - v'
            return differentiate(*sub.lhs, var_name) - differentiate(*sub.rhs, var_name);
        },
        [&var_name](const Mul& mul) -> Expression {
            // Product Rule: (u * v)' = u' * v + u * v'
            Expression u = *mul.lhs;
            Expression v = *mul.rhs;
            Expression u_prime = differentiate(u, var_name);
            Expression v_prime = differentiate(v, var_name);

            return (u_prime * v) + (u * v_prime);
        },
        [&var_name](const Div& div) -> Expression {
            // Quotient Rule: (u / v)' = (u' * v - u * v') / (v ^ 2)
            Expression u = *div.lhs;
            Expression v = *div.rhs;
            Expression u_prime = differentiate(u, var_name);
            Expression v_prime = differentiate(v, var_name);

            return ((u_prime * v) - (u * v_prime)) / pow(v, 2.0);
        },
        [&var_name](const Power& p) -> Expression {
            // Generalized Power Rule: (u ^ n)' = n * (u ^ (n - 1)) * u'
            Expression u = *p.base;
            Expression u_prime = differentiate(u, var_name);

            return num(p.exponent) * pow(u, p.exponent - 1.0) * u_prime;
        }
    }, expr.node);
}

} // namespace sym

#endif // DIFFERENTIATOR_HPP