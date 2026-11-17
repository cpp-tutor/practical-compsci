#include "SymbolicEngine.hpp"
#include "Evaluator.hpp"
#include "Simplifier.hpp"
#include "Differentiator.hpp"
#include <iostream>
#include <format>

int main() {
    std::cout << "=========================================================\n";
    std::cout << "Symbolic Algebraic Engine and Type-Driven Differentiation\n";
    std::cout << "=========================================================\n\n";

    using namespace sym;

    // Define symbolic variables: x and y
    auto x = var("x");
    auto y = var("y");

    // -------------------------------------------------------------
    // Example 1: f(x) = 3x^2 + 5x + 7
    // -------------------------------------------------------------
    Expression f = (num(3.0) * pow(x, 2.0)) + (num(5.0) * x) + num(7.0);

    std::cout << std::format("Original f(x)             : {}\n", to_string(f));

    // Numerical evaluation at x = 4 -> f(4) = 3(16) + 20 + 7 = 75
    Environment env{{"x", 4.0}};
    std::cout << std::format("Evaluate f(4)             : {}\n", evaluate(f, env));

    // Differentiate: f'(x) = 6x + 5
    Expression raw_derivative = differentiate(f, "x");
    std::cout << std::format("Raw Derivative f'(x)      : {}\n", to_string(raw_derivative));

    // Simplify derivative using algebraic reduction rules
    Expression clean_derivative = simplify(raw_derivative);
    std::cout << std::format("Simplified f'(x)          : {}\n", to_string(clean_derivative));

    // Evaluate f'(4) -> 6(4) + 5 = 29
    std::cout << std::format("Evaluate f'(4)            : {}\n\n", evaluate(clean_derivative, env));

    // -------------------------------------------------------------
    // Example 2: Product Rule Demonstration: g(x) = (x^3) * (x + 2)
    // g'(x) = 3x^2(x + 2) + x^3(1) = 4x^3 + 6x^2
    // -------------------------------------------------------------
    Expression g = pow(x, 3.0) * (x + num(2.0));
    std::cout << std::format("Original g(x)             : {}\n", to_string(g));
    Expression g_prime = simplify(differentiate(g, "x"));
    std::cout << std::format("Simplified g'(x)          : {}\n", to_string(g_prime));
    std::cout << std::format("Evaluate g'(2)            : {}\n\n", evaluate(g_prime, Environment{{"x", 2.0}}));

    return 0;
}