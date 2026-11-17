#include "../chapter-18/Lexer.hpp"
#include "Parser.hpp"
#include "ASTVisitor.hpp"
#include <iostream>

void test_parser(std::string_view title, std::string_view source_code) {
    using namespace Faberge;

    std::cout << "\n====================================================\n";
    std::cout << " TEST CASE: " << title << "\n";
    std::cout << "====================================================\n";
    std::cout << "SOURCE CODE:\n" << source_code << "\n----------------------------------------------------\n";

    // 1. Stage 1: Lexical Analysis
    Lexer lexer(source_code, "test.fab");
    auto tokens = lexer.tokenise();

    if (lexer.has_errors()) {
        std::cerr << "[Lexer Error]: Aborting parse due to scan errors.\n";
        return;
    }

    // 2. Stage 2: Syntactic Analysis
    Parser parser(std::move(tokens), source_code);
    auto ast = parser.parse_program();

    if (parser.has_errors()) {
        std::cerr << "\n[Parser Error]: Parsing failed with " << parser.error_count() << " error(s).\n\n";
        return;
    }

    // 3. Render AST
    std::cout << "ABSTRACT SYNTAX TREE (AST):\n";
    ASTPrinter printer;
    printer.print(*ast);
    std::cout << "----------------------------------------------------\n";
    std::cout << "[Success]: AST constructed cleanly.\n\n";
}

int main() {
    // Test 1: Complex Arithmetic and Operator Precedence
    constexpr std::string_view precedence_source = R"(
        let result = 1 + 2 * 3 == 7 && 10 > 5;
    )";
    test_parser("Arithmetic and Precedence Resolution", precedence_source);

    // Test 2: Recursive Functional Pipeline with Lambda and Block Expression
    constexpr std::string_view functional_source = R"(
        let factorial = fn (n: Int): Int => {
            if n <= 1 then 1 else n * factorial(n - 1)
        };

        let total = factorial(5) + 100;
    )";
    test_parser("Higher-Order Functional Pipeline", functional_source);

    // Test 3: Parser Error Diagnostics and Panic-Mode Recovery
    constexpr std::string_view broken_source = R"(
        let valid_a = 10;
        let broken_b = ; // Missing initialiser expression!
        let valid_c = 30 + 40;
    )";
    test_parser("Panic-Mode Error Recovery", broken_source);

    return 0;
}