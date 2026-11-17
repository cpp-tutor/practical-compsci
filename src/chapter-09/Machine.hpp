#ifndef MACHINE_HPP
#define MACHINE_HPP

#include "Mach-16.hpp"
#include <iostream>
#include <iomanip>

namespace mach16 {

class VirtualMachine {
public:
    VirtualMachine() noexcept {
        reset();
    }

    void reset() noexcept {
        m_memory.fill(0);
        m_pc = 0;
        m_ir = 0;
        m_acc = 0;
        m_mar = 0;
        m_mdr = 0;
        m_flags.reset();
        m_cycle_count = 0;
    }

    // Load a program into memory at a specified base offset
    void load_program(const std::vector<std::uint16_t>& program, std::uint16_t origin = 0) {
        if (origin + program.size() > MEMORY_SIZE) {
            throw std::out_of_range("Program binary exceeds physical memory bounds.");
        }
        for (std::size_t i = 0; i < program.size(); ++i) {
            m_memory[origin + i] = program[i];
        }
        m_pc = origin;
    }

    // Direct memory inspection/injection
    void write_memory(std::uint16_t address, std::uint16_t value) {
        m_memory[address & ADDRESS_MASK] = value;
    }

    [[nodiscard]] std::uint16_t read_memory(std::uint16_t address) const noexcept {
        return m_memory[address & ADDRESS_MASK];
    }

    // Execute a single Fetch-Decode-Execute cycle
    bool step(bool trace = false) {
        if (m_flags.halt) {
            return false;
        }

        // =========================================================
        // 1. FETCH STAGE
        // =========================================================
        m_mar = m_pc;
        m_mdr = m_memory[m_mar];
        m_ir = m_mdr;
        const std::uint16_t fetched_address = m_pc;
        m_pc = (m_pc + 1) & ADDRESS_MASK;

        // =========================================================
        // 2. DECODE STAGE
        // =========================================================
        const Instruction instr = Instruction::decode(m_ir);

        // Resolve operand according to Addressing Mode
        std::int16_t resolved_value = 0;
        std::uint16_t target_address = 0;

        if (instr.mode == AddressingMode::Immediate) {
            // Sign-extend 10-bit immediate if treating as signed integer
            // For simplicity, we zero-extend immediate literals 0..1023
            resolved_value = static_cast<std::int16_t>(instr.operand);
        } else if (instr.mode == AddressingMode::Direct) {
            target_address = instr.operand;
            m_mar = target_address;
            m_mdr = m_memory[m_mar];
            resolved_value = static_cast<std::int16_t>(m_mdr);
        } else if (instr.mode == AddressingMode::Indirect) {
            // Memory points to another address
            m_mar = instr.operand;
            m_mdr = m_memory[m_mar];
            target_address = m_mdr & ADDRESS_MASK;
            m_mar = target_address;
            m_mdr = m_memory[m_mar];
            resolved_value = static_cast<std::int16_t>(m_mdr);
        }

        if (trace) {
            log_trace(fetched_address, instr, resolved_value);
        }

        // =========================================================
        // 3. EXECUTE STAGE
        // =========================================================
        execute_instruction(instr, resolved_value, target_address);

        ++m_cycle_count;
        return !m_flags.halt;
    }

    // Run continuously until HLT or maximum cycle limit reached
    void run(std::size_t max_cycles = 1'000'000, bool trace = false) {
        while (!m_flags.halt && m_cycle_count < max_cycles) {
            step(trace);
        }
    }

    // Diagnostic Dump
    void dump_state() const {
        std::cout << "\n========== MACH-16 CPU STATE DUMP ==========\n";
        std::cout << std::format("Cycles Run : {}\n", m_cycle_count);
        std::cout << std::format("PC         : 0x{:04X} ({})\n", m_pc, m_pc);
        std::cout << std::format("IR         : 0x{:04X}\n", m_ir);
        std::cout << std::format("ACC (Hex)  : 0x{:04X}\n", static_cast<std::uint16_t>(m_acc));
        std::cout << std::format("ACC (Dec)  : {}\n", m_acc);
        std::cout << std::format("Flags      : [Z: {} | N: {} | C: {} | H: {}]\n",
                                 m_flags.zero ? 1 : 0,
                                 m_flags.negative ? 1 : 0,
                                 m_flags.carry ? 1 : 0,
                                 m_flags.halt ? 1 : 0);
        std::cout << "==========================================\n";
    }

    [[nodiscard]] std::int16_t get_acc() const noexcept { return m_acc; }
    [[nodiscard]] const Flags& get_flags() const noexcept { return m_flags; }
    [[nodiscard]] std::size_t get_cycle_count() const noexcept { return m_cycle_count; }

private:
    void update_arithmetic_flags(std::int32_t result) noexcept {
        m_acc = static_cast<std::int16_t>(result & 0xFFFF);
        m_flags.zero = (m_acc == 0);
        m_flags.negative = (m_acc < 0);
        m_flags.carry = (result > 32767 || result < -32768);
    }

    void update_logic_flags() noexcept {
        m_flags.zero = (m_acc == 0);
        m_flags.negative = (m_acc < 0);
        m_flags.carry = false; // Logic operations clear carry
    }

    void execute_instruction(const Instruction& instr, std::int16_t value, std::uint16_t target_addr) {
        switch (instr.opcode) {
            case Opcode::HLT:
                m_flags.halt = true;
                break;

            case Opcode::NOP:
                // No action performed
                break;

            case Opcode::LDA:
                m_acc = value;
                m_flags.zero = (m_acc == 0);
                m_flags.negative = (m_acc < 0);
                break;

            case Opcode::STA:
                m_mar = (instr.mode == AddressingMode::Indirect) ? target_addr : instr.operand;
                m_mdr = static_cast<std::uint16_t>(m_acc);
                m_memory[m_mar & ADDRESS_MASK] = m_mdr;
                break;

            case Opcode::ADD: {
                const std::int32_t res = static_cast<std::int32_t>(m_acc) + static_cast<std::int32_t>(value);
                update_arithmetic_flags(res);
                break;
            }

            case Opcode::SUB: {
                const std::int32_t res = static_cast<std::int32_t>(m_acc) - static_cast<std::int32_t>(value);
                update_arithmetic_flags(res);
                break;
            }

            case Opcode::MUL: {
                const std::int32_t res = static_cast<std::int32_t>(m_acc) * static_cast<std::int32_t>(value);
                update_arithmetic_flags(res);
                break;
            }

            case Opcode::DIV: {
                if (value == 0) {
                    throw std::runtime_error("Hardware Trap: Integer division by zero.");
                }
                const std::int32_t res = static_cast<std::int32_t>(m_acc) / static_cast<std::int32_t>(value);
                update_arithmetic_flags(res);
                break;
            }

            case Opcode::AND:
                m_acc = static_cast<std::int16_t>(m_acc & value);
                update_logic_flags();
                break;

            case Opcode::OR:
                m_acc = static_cast<std::int16_t>(m_acc | value);
                update_logic_flags();
                break;

            case Opcode::XOR:
                m_acc = static_cast<std::int16_t>(m_acc ^ value);
                update_logic_flags();
                break;

            case Opcode::NOT:
                m_acc = static_cast<std::int16_t>(~m_acc);
                update_logic_flags();
                break;

            case Opcode::JMP:
                m_pc = instr.operand & ADDRESS_MASK;
                break;

            case Opcode::JZ:
                if (m_flags.zero) {
                    m_pc = instr.operand & ADDRESS_MASK;
                }
                break;

            case Opcode::JNZ:
                if (!m_flags.zero) {
                    m_pc = instr.operand & ADDRESS_MASK;
                }
                break;

            case Opcode::JN:
                if (m_flags.negative) {
                    m_pc = instr.operand & ADDRESS_MASK;
                }
                break;
        }
    }

    void log_trace(std::uint16_t fetch_pc, const Instruction& instr, std::int16_t resolved_val) const {
        std::string mode_str;
        switch (instr.mode) {
            case AddressingMode::Direct:    mode_str = "[DIR]"; break;
            case AddressingMode::Immediate: mode_str = "[IMM]"; break;
            case AddressingMode::Indirect:  mode_str = "[IND]"; break;
        }

        std::cout << std::format("CYC:{:04d} | PC:0x{:03X} | RAW:0x{:04X} | {} {:<4} {:<4} (Val:{:>5}) | ACC:{:>6} | [Z:{} N:{} C:{}]\n",
            m_cycle_count,
            fetch_pc,
            m_ir,
            mode_str,
            opcode_to_string(instr.opcode),
            instr.operand,
            resolved_val,
            m_acc,
            m_flags.zero ? 1 : 0,
            m_flags.negative ? 1 : 0,
            m_flags.carry ? 1 : 0
        );
    }

    // Hardware State
    std::array<std::uint16_t, MEMORY_SIZE> m_memory{};
    std::uint16_t m_pc{0};
    std::uint16_t m_ir{0};
    std::int16_t  m_acc{0};
    std::uint16_t m_mar{0};
    std::uint16_t m_mdr{0};
    Flags         m_flags{};
    std::size_t   m_cycle_count{0};
};

} // namespace mach16

#endif // MACHINE_HPP