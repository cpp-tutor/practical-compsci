#ifndef ASSEMBLER_HPP
#define ASSEMBLER_HPP

#include "Mach-16.hpp"
#include <unordered_map>
#include <sstream>
#include <algorithm>

namespace mach16 {

class Assembler {
public:
    static std::vector<std::uint16_t> assemble(std::string_view source_code) {
        std::unordered_map<std::string, std::uint16_t> symbol_table;
        std::vector<std::string> lines = split_lines(source_code);

        // =========================================================
        // PASS 1: Symbol Identification
        // =========================================================
        std::uint16_t current_address = 0;
        for (const auto& raw_line : lines) {
            std::string line = strip_comments(raw_line);
            line = trim(line);
            if (line.empty()) continue;

            if (line.ends_with(':')) {
                // Label definition
                std::string label_name = line.substr(0, line.length() - 1);
                label_name = trim(label_name);
                if (symbol_table.contains(label_name)) {
                    throw std::runtime_error(std::format("Duplicate label: {}", label_name));
                }
                symbol_table[label_name] = current_address;
            } else {
                // Instruction or raw data directive
                ++current_address;
            }
        }

        // =========================================================
        // PASS 2: Code Generation
        // =========================================================
        std::vector<std::uint16_t> bytecode;
        current_address = 0;

        for (const auto& raw_line : lines) {
            std::string line = strip_comments(raw_line);
            line = trim(line);
            if (line.empty() || line.ends_with(':')) continue;

            std::stringstream ss(line);
            std::string mnemonic;
            ss >> mnemonic;
            std::transform(mnemonic.begin(), mnemonic.end(), mnemonic.begin(), ::toupper);

            // Special Pseudo-op: .WORD (stores a raw integer constant)
            if (mnemonic == ".WORD") {
                std::string literal_str;
                ss >> literal_str;
                std::uint16_t raw_val = parse_literal_or_symbol(literal_str, symbol_table);
                bytecode.push_back(raw_val);
                ++current_address;
                continue;
            }

            Opcode op = parse_opcode(mnemonic);
            AddressingMode mode = AddressingMode::Direct;
            std::uint16_t operand = 0;

            if (op == Opcode::HLT || op == Opcode::NOP || op == Opcode::NOT) {
                // No operands required
                bytecode.push_back(Instruction{op, AddressingMode::Direct, 0}.encode());
                ++current_address;
                continue;
            }

            std::string token;
            if (ss >> token) {
                if (token.starts_with('#')) {
                    mode = AddressingMode::Immediate;
                    operand = parse_literal_or_symbol(token.substr(1), symbol_table);
                } else if (token.starts_with('@')) {
                    mode = AddressingMode::Indirect;
                    operand = parse_literal_or_symbol(token.substr(1), symbol_table);
                } else {
                    mode = AddressingMode::Direct;
                    operand = parse_literal_or_symbol(token, symbol_table);
                }
            } else {
                throw std::runtime_error(std::format("Instruction {} requires an operand.", mnemonic));
            }

            bytecode.push_back(Instruction{op, mode, operand}.encode());
            ++current_address;
        }

        return bytecode;
    }

private:
    static std::vector<std::string> split_lines(std::string_view text) {
        std::vector<std::string> result;
        std::stringstream ss{std::string(text)};
        std::string line;
        while (std::getline(ss, line)) {
            result.push_back(line);
        }
        return result;
    }

    static std::string strip_comments(std::string_view line) {
        auto comment_pos = line.find(';');
        if (comment_pos != std::string_view::npos) {
            line = line.substr(0, comment_pos);
        }
        return std::string(line);
    }

    static std::string trim(std::string_view str) {
        const auto start = str.find_first_not_of(" \t\r\n");
        if (start == std::string_view::npos) return "";
        const auto end = str.find_last_not_of(" \t\r\n");
        return std::string(str.substr(start, end - start + 1));
    }

    static Opcode parse_opcode(std::string_view m) {
        if (m == "HLT") return Opcode::HLT;
        if (m == "NOP") return Opcode::NOP;
        if (m == "LDA") return Opcode::LDA;
        if (m == "STA") return Opcode::STA;
        if (m == "ADD") return Opcode::ADD;
        if (m == "SUB") return Opcode::SUB;
        if (m == "MUL") return Opcode::MUL;
        if (m == "DIV") return Opcode::DIV;
        if (m == "AND") return Opcode::AND;
        if (m == "OR")  return Opcode::OR;
        if (m == "XOR") return Opcode::XOR;
        if (m == "NOT") return Opcode::NOT;
        if (m == "JMP") return Opcode::JMP;
        if (m == "JZ")  return Opcode::JZ;
        if (m == "JNZ") return Opcode::JNZ;
        if (m == "JN")  return Opcode::JN;
        throw std::runtime_error(std::format("Unknown mnemonic: {}", m));
    }

    static std::uint16_t parse_literal_or_symbol(
        const std::string& token,
        const std::unordered_map<std::string, std::uint16_t>& symbols)
    {
        if (symbols.contains(token)) {
            return symbols.at(token);
        }
        try {
            if (token.starts_with("0x") || token.starts_with("0X")) {
                return static_cast<std::uint16_t>(std::stoul(token, nullptr, 16));
            }
            return static_cast<std::uint16_t>(std::stoul(token, nullptr, 10));
        } catch (...) {
            throw std::runtime_error(std::format("Failed to resolve symbol or literal: {}", token));
        }
    }
};

} // namespace mach16

#endif // ASSEMBLER_HPP