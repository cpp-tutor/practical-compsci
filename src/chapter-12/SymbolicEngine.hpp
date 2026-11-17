#ifndef SYMBOLIC_ENGINE_HPP
#define SYMBOLIC_ENGINE_HPP

#include <memory>
#include <string>
#include <variant>
#include <iostream>
#include <format>
#include <cmath>
#include <stdexcept>
#include <concepts>

namespace sym {

// Forward declaration of recursive AST Node
struct Expression;

// Pattern Matcher Helper
template <class... Ts>
struct overloaded : Ts... {
    using Ts::operator()...;
};
template <class... Ts>
overloaded(Ts...) -> overloaded<Ts...>;

// =========================================================================
// 1. AST Node Primitives (Sum Type Constituents)
// =========================================================================

// Numeric literal: 42, 3.1415
struct Number {
    double value;
};

// Named variable: "x", "y", "theta"
struct Variable {
    std::string name;
};

// Unary negation: -(expr)
struct Negate {
    std::shared_ptr<Expression> target;
};

// Binary addition: (lhs + rhs)
struct Add {
    std::shared_ptr<Expression> lhs;
    std::shared_ptr<Expression> rhs;
};

// Binary subtraction: (lhs - rhs)
struct Sub {
    std::shared_ptr<Expression> lhs;
    std::shared_ptr<Expression> rhs;
};

// Binary multiplication: (lhs * rhs)
struct Mul {
    std::shared_ptr<Expression> lhs;
    std::shared_ptr<Expression> rhs;
};

// Binary division: (lhs / rhs)
struct Div {
    std::shared_ptr<Expression> lhs;
    std::shared_ptr<Expression> rhs;
};

// Exponentiation: (base ^ exponent)
struct Power {
    std::shared_ptr<Expression> base;
    double exponent;
};

// =========================================================================
// 2. The Core Algebraic Sum Type
// =========================================================================

struct Expression {
    std::variant<
        Number,
        Variable,
        Negate,
        Add,
        Sub,
        Mul,
        Div,
        Power
    > node;

    // Constructors for seamless AST creation
    Expression(Number n) : node(n) {}
    Expression(Variable v) : node(std::move(v)) {}
    Expression(Negate neg) : node(std::move(neg)) {}
    Expression(Add add) : node(std::move(add)) {}
    Expression(Sub sub) : node(std::move(sub)) {}
    Expression(Mul mul) : node(std::move(mul)) {}
    Expression(Div div) : node(std::move(div)) {}
    Expression(Power pow) : node(std::move(pow)) {}
};

// Helper factory functions
[[nodiscard]] inline Expression num(double v) { return Expression{Number{v}}; }
[[nodiscard]] inline Expression var(std::string name) { return Expression{Variable{std::move(name)}}; }

[[nodiscard]] inline Expression operator+(Expression lhs, Expression rhs) {
    return Expression{Add{std::make_shared<Expression>(std::move(lhs)), std::make_shared<Expression>(std::move(rhs))}};
}

[[nodiscard]] inline Expression operator-(Expression lhs, Expression rhs) {
    return Expression{Sub{std::make_shared<Expression>(std::move(lhs)), std::make_shared<Expression>(std::move(rhs))}};
}

[[nodiscard]] inline Expression operator*(Expression lhs, Expression rhs) {
    return Expression{Mul{std::make_shared<Expression>(std::move(lhs)), std::make_shared<Expression>(std::move(rhs))}};
}

[[nodiscard]] inline Expression operator/(Expression lhs, Expression rhs) {
    return Expression{Div{std::make_shared<Expression>(std::move(lhs)), std::make_shared<Expression>(std::move(rhs))}};
}

[[nodiscard]] inline Expression operator-(Expression expr) {
    return Expression{Negate{std::make_shared<Expression>(std::move(expr))}};
}

[[nodiscard]] inline Expression pow(Expression base, double exp) {
    return Expression{Power{std::make_shared<Expression>(std::move(base)), exp}};
}

} // namespace sym

#endif // SYMBOLIC_ENGINE_HPP