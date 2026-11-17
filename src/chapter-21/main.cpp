#include "../chapter-18/Lexer.hpp"
#include "../chapter-19/Parser.hpp"
#include "../chapter-20/TypeChecker.hpp"
#include "Emitter.hpp"

#include <iostream>
#include <fstream>
#include <sstream>

namespace Faberge {

class Compiler {
public:
    static bool compile_source(std::string_view source_code, 
                               std::string_view filename, 
                               std::ostream& output_stream) {
        std::cout << "\033[1;34m[Faberge Compiler]\033[0m Compiling '" << filename << "'...\n";

        // STAGE 1: Lexical Analysis
        Lexer lexer(source_code, filename);
        auto tokens = lexer.tokenise();
        if (lexer.has_errors()) {
            std::cerr << "\033[1;31m[Compilation Aborted]\033[0m Lexical errors detected.\n";
            return false;
        }
        std::cout << "  ✓ Stage 1: Lexical Analysis completed (" << tokens.size() << " tokens).\n";

        // STAGE 2: Syntactic Analysis (Parser)
        Parser parser(std::move(tokens), source_code);
        auto ast = parser.parse_program();
        if (parser.has_errors()) {
            std::cerr << "\033[1;31m[Compilation Aborted]\033[0m Syntax errors detected.\n";
            return false;
        }
        std::cout << "  ✓ Stage 2: Syntactic Analysis completed (AST constructed).\n";

        // STAGE 3: Semantic Analysis and Static Type Checking
        TypeChecker checker(source_code);
        if (!checker.check(*ast)) {
            std::cerr << "\033[1;31m[Compilation Aborted]\033[0m Semantic type verification failed.\n";
            return false;
        }
        std::cout << "  ✓ Stage 3: Semantic Analysis completed (Type Safety Proved).\n";

        // STAGE 4: Code Generation (C++ Target Emission)
        CppEmitter emitter;
        std::string cpp_code = emitter.generate(*ast);
        std::cout << "  ✓ Stage 4: Target Code Generation completed.\n";

        output_stream << cpp_code;
        return true;
    }
};

} // namespace Faberge

int main() {
    // A complete, expressive functional Faberge program
    constexpr std::string_view faberge_source = R"(
        // --- Mathematical Helper Functions ---
        let square = fn (x: Int): Int => x * x;

        let add = fn (a: Int, b: Int): Int => a + b;

        // Recursive Factorial Implementation
        let factorial = fn (n: Int): Int => {
            if n <= 1 then 
                1 
            else 
                n * factorial(n - 1)
        };

        // Recursive Fibonacci Implementation
        let fibonacci = fn (n: Int): Int => {
            if n <= 0 then
                0
            else if n == 1 then
                1
            else
                fibonacci(n - 1) + fibonacci(n - 2)
        };

        // Main Program Entry Point
        let main = fn (): Int => {
            let num = 5;
            let fac5 = factorial(num);
            let fib7 = fibonacci(7);
            
            // Block Expression with Lexical Scoping
            let combined = {
                let multiplier = 10;
                let squared = square(fac5);
                (squared / multiplier) + fib7
            };

            combined
        };
    )";

    std::cout << "====================================================\n";
    std::cout << "      The Faberge Compiler (Complete Demo)    \n";
    std::cout << "====================================================\n\n";

    std::ostringstream generated_cpp;
    bool success = Faberge::Compiler::compile_source(faberge_source, "sample.fab", generated_cpp);

    if (success) {
        std::cout << "\n\033[1;32m[SUCCESS] Compilation Succeeded!\033[0m\n";
        std::cout << "====================================================\n";
        std::cout << "EMITTED C++ CODE:\n";
        std::cout << "====================================================\n";
        std::cout << generated_cpp.str() << "\n";
        std::cout << "====================================================\n";
    } else {
        std::cout << "\n\033[1;31m[FAILURE] Compilation Failed.\033[0m\n";
        return 1;
    }

    return 0;
}