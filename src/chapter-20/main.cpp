#include "../chapter-18/Lexer.hpp"
#include "../chapter-19/Parser.hpp"
#include "TypeChecker.hpp"
#include <iostream>

void run_semantic_test(std::string_view title, std::string_view source_code) {
    using namespace Faberge;

    std::cout << "\n====================================================\n";
    std::cout << " TEST CASE: " << title << "\n";
    std::cout << "====================================================\n";
    std::cout << "SOURCE CODE:\n" << source_code << "\n----------------------------------------------------\n";

    // Stage 1: Lexical Analysis
    Lexer lexer(source_code, "test.fab");
    auto tokens = lexer.tokenise();
    if (lexer.has_errors()) {
        std::cerr << "[Abort]: Lexer errors encountered.\n";
        return;
    }

    // Stage 2: Syntactic Analysis (Parser)
    Parser parser(std::move(tokens), source_code);
    auto ast = parser.parse_program();
    if (parser.has_errors()) {
        std::cerr << "[Abort]: Parser errors encountered.\n";
        return;
    }

    // Stage 3: Semantic Analysis and Type Checking
    TypeChecker checker(source_code);
    bool passed = checker.check(*ast);

    if (passed) {
        std::cout << "\033[1;32m[PASS]: Static Semantic and Type Verification Succeeded (0 Errors).\033[0m\n";
    } else {
        std::cout << "\033[1;31m[FAIL]: Semantic Analysis Failed with " 
                  << checker.error_count() << " error(s).\033[0m\n";
    }
}

int main() {
    // 1. Valid Higher-Order Recursive Program
    constexpr std::string_view valid_source = R"(
        let add = fn (a: Int, b: Int): Int => a + b;

        let factorial = fn (n: Int): Int => {
            if n <= 1 then 1 else n * factorial(n - 1)
        };

        let main = fn (): Int => {
            let x = 10;
            let result = factorial(x);
            add(result, 42)
        };
    )";
    run_semantic_test("Valid Recursive Functional Program", valid_source);

    // 2. Intentional Semantic Violations
    constexpr std::string_view invalid_source = R"(
        let bad_math = 10 + "hello"; // Error 1: Type mismatch in addition

        let bad_branch = if true then 42 else "divergent string"; // Error 2: Branch type mismatch

        let bad_call = fn (x: Int): Int => {
            let local_flag = true;
            undeclared_function(x, local_flag) // Error 3: Undeclared identifier
        };

        let return_mismatch = fn (a: Int): Int => {
            "this is not an integer" // Error 4: Return type mismatch
        };
    )";
    //run_semantic_test("Negative Semantic Tests (Intentional Errors)", invalid_source);

    return 0;
}