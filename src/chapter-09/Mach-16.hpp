#ifndef MACH16_HPP
#define MACH16_HPP

#include <cstdint>
#include <array>
#include <string>
#include <vector>
#include <string_view>
#include <optional>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <format>

namespace mach16 {

// The Mach-16 physical parameters
constexpr std::size_t MEMORY_SIZE = 1024; // 1024 16-bit words
constexpr std::uint16_t ADDRESS_MASK = 0x03FF; // 10-bit address space

enum class Opcode : std::uint8_t {
    HLT = 0x0,
    NOP = 0x1,
    LDA = 0x2,
    STA = 0x3,
    ADD = 0x4,
    SUB = 0x5,
    MUL = 0x6,
    DIV = 0x7,
    AND = 0x8,
    OR  = 0x9,
    XOR = 0xA,
    NOT = 0xB,
    JMP = 0xC,
    JZ  = 0xD,
    JNZ = 0xE,
    JN  = 0xF
};

enum class AddressingMode : std::uint8_t {
    Direct    = 0x0, // Address refers to memory location
    Immediate = 0x1, // Operand is a raw numeric literal
    Indirect  = 0x2  // Address refers to a pointer cell in memory
};

struct Flags {
    bool zero{false};
    bool negative{false};
    bool carry{false};
    bool halt{false};

    void reset() noexcept {
        zero = false;
        negative = false;
        carry = false;
        halt = false;
    }
};

// Decoded instruction representation
struct Instruction {
    Opcode opcode;
    AddressingMode mode;
    std::uint16_t operand; // 10 bits

    // Pack instruction into a raw 16-bit machine word
    [[nodiscard]] constexpr std::uint16_t encode() const noexcept {
        const auto op = static_cast<std::uint16_t>(opcode) & 0x0F;
        const auto md = static_cast<std::uint16_t>(mode) & 0x03;
        const auto opr = operand & ADDRESS_MASK;
        return static_cast<std::uint16_t>((op << 12) | (md << 10) | opr);
    }

    // Decode a raw 16-bit machine word into an Instruction
    [[nodiscard]] static constexpr Instruction decode(std::uint16_t word) noexcept {
        const auto op = static_cast<Opcode>((word >> 12) & 0x0F);
        const auto md = static_cast<AddressingMode>((word >> 10) & 0x03);
        const auto opr = static_cast<std::uint16_t>(word & ADDRESS_MASK);
        return Instruction{op, md, opr};
    }
};

// Convert opcode to human-readable string
[[nodiscard]] inline std::string_view opcode_to_string(Opcode op) noexcept {
    switch (op) {
        case Opcode::HLT: return "HLT";
        case Opcode::NOP: return "NOP";
        case Opcode::LDA: return "LDA";
        case Opcode::STA: return "STA";
        case Opcode::ADD: return "ADD";
        case Opcode::SUB: return "SUB";
        case Opcode::MUL: return "MUL";
        case Opcode::DIV: return "DIV";
        case Opcode::AND: return "AND";
        case Opcode::OR:  return "OR";
        case Opcode::XOR: return "XOR";
        case Opcode::NOT: return "NOT";
        case Opcode::JMP: return "JMP";
        case Opcode::JZ:  return "JZ";
        case Opcode::JNZ: return "JNZ";
        case Opcode::JN:  return "JN";
        default:          return "UNKNOWN";
    }
}

} // namespace mach16

#endif // MACH16_HPP