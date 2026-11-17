#include "Lexer.hpp"
#include <iostream>
#include <iomanip>

void run_lexer_demo(std::string_view test_name, std::string_view source_code) {
    using namespace Faberge;

    std::cout << "\n====================================================\n";
    std::cout << " TEST CASE: " << test_name << "\n";
    std::cout << "====================================================\n";
    std::cout << "SOURCE CODE:\n" << source_code << "\n----------------------------------------------------\n";

    Lexer lexer(source_code, "test.fab");
    auto tokens = lexer.tokenise();

    if (lexer.has_errors()) {
        std::cout << "[Result]: Lexer encountered " << lexer.error_count() << " error(s).\n\n";
        return;
    }

    std::cout << std::left << std::setw(6)  << "LINE" 
              << std::left << std::setw(6)  << "COL" 
              << std::left << std::setw(22) << "TOKEN TYPE" 
              << "LEXEME\n";
    std::cout << "----------------------------------------------------\n";

    for (const auto& token : tokens) {
        if (token.type == TokenType::EndOfFile) break;
        std::cout << std::left << std::setw(6)  << token.location.line
                  << std::left << std::setw(6)  << token.location.column
                  << std::left << std::setw(22) << Token::to_string(token.type)
                  << "'" << token.lexeme << "'\n";
    }
    std::cout << "----------------------------------------------------\n";
    std::cout << "[Success]: Emitted " << tokens.size() << " tokens cleanly.\n\n";
}

int main() {
    // 1. Valid Functional Script: Factorial and Higher-Order Functions
    constexpr std::string_view valid_source = R"(
        /* 
           Higher-order recursive pipeline
           Demonstrating nested block comments: /* valid nested block */ 
        */
        let factorial = fn (n: Int): Int => {
            if n <= 1 then 1 else n * factorial(n - 1)
        };

        let main = fn (): Int => {
            let limit = 42;
            let pi_approx = 3.14159;
            let greeting = "Hello, Faberge Compiler!\n";
            factorial(5)
        };
    )";

    run_lexer_demo("Valid Functional Script", valid_source);

    // 2. Syntax with Errors: Invalid escape sequence and unclosed comment
    constexpr std::string_view error_source = R"(
        let bad_string = "Invalid escape: \q here";
        let unclosed_comment = /* Unfinished block comment...
        let x = 100;
    )";

    run_lexer_demo("Syntax Error Diagnostics", error_source);

    return 0;
}