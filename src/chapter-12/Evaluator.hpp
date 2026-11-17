#ifndef EVALUATOR_HPP
#define EVALUATOR_HPP

#include "SymbolicEngine.hpp"
#include <unordered_map>

namespace sym {

// Pretty Print Expression to Standard Mathematical Notation
[[nodiscard]] inline std::string to_string(const Expression& expr) {
    return std::visit(overloaded{
        [](const Number& n) -> std::string {
            if (n.value == static_cast<long long>(n.value)) {
                return std::format("{}", static_cast<long long>(n.value));
            }
            return std::format("{:.2f}", n.value);
        },
        [](const Variable& v) -> std::string {
            return v.name;
        },
        [](const Negate& neg) -> std::string {
            return std::format("-({})", to_string(*neg.target));
        },
        [](const Add& add) -> std::string {
            return std::format("({} + {})", to_string(*add.lhs), to_string(*add.rhs));
        },
        [](const Sub& sub) -> std::string {
            return std::format("({} - {})", to_string(*sub.lhs), to_string(*sub.rhs));
        },
        [](const Mul& mul) -> std::string {
            return std::format("({} * {})", to_string(*mul.lhs), to_string(*mul.rhs));
        },
        [](const Div& div) -> std::string {
            return std::format("({} / {})", to_string(*div.lhs), to_string(*div.rhs));
        },
        [](const Power& p) -> std::string {
            return std::format("({} ^ {})", to_string(*p.base), p.exponent);
        }
    }, expr.node);
}

// Numerical Evaluator over Environment Dictionary
using Environment = std::unordered_map<std::string, double>;

[[nodiscard]] inline double evaluate(const Expression& expr, const Environment& env) {
    return std::visit(overloaded{
        [](const Number& n) -> double {
            return n.value;
        },
        [&env](const Variable& v) -> double {
            if (!env.contains(v.name)) {
                throw std::runtime_error(std::format("Unbound variable in environment: '{}'", v.name));
            }
            return env.at(v.name);
        },
        [&env](const Negate& neg) -> double {
            return -evaluate(*neg.target, env);
        },
        [&env](const Add& add) -> double {
            return evaluate(*add.lhs, env) + evaluate(*add.rhs, env);
        },
        [&env](const Sub& sub) -> double {
            return evaluate(*sub.lhs, env) - evaluate(*sub.rhs, env);
        },
        [&env](const Mul& mul) -> double {
            return evaluate(*mul.lhs, env) * evaluate(*mul.rhs, env);
        },
        [&env](const Div& div) -> double {
            const double denom = evaluate(*div.rhs, env);
            if (denom == 0.0) {
                throw std::runtime_error("Evaluation Exception: Math division by zero.");
            }
            return evaluate(*div.lhs, env) / denom;
        },
        [&env](const Power& p) -> double {
            return std::pow(evaluate(*p.base, env), p.exponent);
        }
    }, expr.node);
}

} // namespace sym

#endif // EVALUATOR_HPP