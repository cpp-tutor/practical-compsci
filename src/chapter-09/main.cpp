#include "Mach-16.hpp"
#include "Machine.hpp"
#include "Assembler.hpp"
#include <iostream>

int main() {
    const std::string factorial_program = R"(
        LDA     VAL_N
        STA     COUNTER
        LDA     #1
        STA     RESULT

    LOOP:
        LDA     COUNTER
        SUB     #1
        JZ      DONE
        
        LDA     RESULT
        MUL     COUNTER
        STA     RESULT

        LDA     COUNTER
        SUB     #1
        STA     COUNTER
        
        JMP     LOOP

    DONE:
        LDA     RESULT
        HLT

    VAL_N:
        .WORD 5
    COUNTER:
        .WORD 0
    RESULT:
        .WORD 0
    )";

    try {
        std::cout << "--- Assembling Program ---\n";
        std::vector<std::uint16_t> binary = mach16::Assembler::assemble(factorial_program);
        std::cout << std::format("Assembled {} machine instructions/words.\n\n", binary.size());

        std::cout << "--- Executing on Mach-16 Virtual Machine (Cycle Trace) ---\n";
        mach16::VirtualMachine vm;
        vm.load_program(binary);
        
        // Run with live micro-architectural trace logging
        vm.run(1000, true);

        // Print final register and memory state
        vm.dump_state();

        std::cout << std::format("Verification: Factorial Result = {}\n", vm.get_acc());
        if (vm.get_acc() == 120) {
            std::cout << ">> SUCCESS: Result matches mathematical expectation (5! = 120).\n";
        } else {
            std::cerr << ">> ERROR: Unexpected computational result.\n";
        }

    } catch (const std::exception& e) {
        std::cerr << "Fatal Exception: " << e.what() << '\n';
        return 1;
    }

    return 0;
}