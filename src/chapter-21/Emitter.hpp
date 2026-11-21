#ifndef EMITTER_HPP
#define EMITTER_HPP

#include "../chapter-19/AST.hpp"
#include "../chapter-19/ASTVisitor.hpp"
#include <sstream>
#include <string>
#include <string_view>
#include <iostream>

namespace Faberge {

class CppEmitter : public ASTVisitor {
public:
    CppEmitter() = default;

    std::string generate(const ProgramAST& program) {
        m_buffer.str("");
        m_buffer.clear();
        m_indent_level = 0;

        // 1. Emit Compiler Header & Preamble
        emit_line("// =============================================================================");
        emit_line("// Generated automatically by the Faberge Compiler (Stage 4 Target Code Emitter)");
        emit_line("// Target: Modern C++23");
        emit_line("// =============================================================================\n");
        emit_line("#include <iostream>");
        emit_line("#include <string>\n");
        emit_line("int main() {");
        indent();

        // 2. Traverse AST and Emit Code
        program.accept(*this);

        // 3. Emit Main Entry-Point Execution Driver
        if (m_has_main) {
            emit_line("\nstd::cout << \"[Faberge Runtime] Program terminated with output: \" << main_() << '\\n';\n");
        }
        emit_line("return 0;");
        dedent();
        emit_line("}");

        return m_buffer.str();
    }

    // -------------------------------------------------------------------------
    // AST Visitor Implementations
    // -------------------------------------------------------------------------

    void visit(const ProgramAST& node) override {
        for (const auto& stmt : node.statements()) {
            stmt->accept(*this);
        }
    }

    void visit(const LetStmtAST& node) override {
        emit_indent();
        m_buffer << "const ";

        if (node.type_annotation().has_value()) {
            m_buffer << map_type(*node.type_annotation()) << " ";
        }
        else {
            m_buffer << "auto ";
        }

        if (node.name() == "main") {
            m_buffer << "main_ = ";
            m_has_main = true;
        }
        else {
            m_buffer << node.name() << " = ";
        }

        std::string prev_self = m_self;
        m_self = node.name();

        node.initialiser().accept(*this);
        m_buffer << ";\n";

        m_self = prev_self;
    }

    void visit(const ExprStmtAST& node) override {
        emit_indent();
        node.expression().accept(*this);
        m_buffer << ";\n";
    }

    void visit(const LiteralExprAST& node) override {
        if (node.token_type() == TokenType::Literal_String) {
            m_buffer << "\"" << escape_string(node.value()) << "\"";
        } else {
            m_buffer << node.value();
        }
    }

    void visit(const VariableExprAST& node) override {
        if (node.name() == m_self) {
            m_buffer << "self_";
        }
        else {
            m_buffer << node.name();
        }
    }

    void visit(const UnaryExprAST& node) override {
        m_buffer << "(";
        if (node.op() == TokenType::Minus) m_buffer << "-";
        else if (node.op() == TokenType::Exclamation) m_buffer << "!";
        
        node.operand().accept(*this);
        m_buffer << ")";
    }

    void visit(const BinaryExprAST& node) override {
        m_buffer << "(";
        node.left().accept(*this);
        m_buffer << " " << map_operator(node.op()) << " ";
        node.right().accept(*this);
        m_buffer << ")";
    }

    void visit(const IfExprAST& node) override {
        // Translate functional if-then-else to C++ ternary expression
        m_buffer << "(";
        node.condition().accept(*this);
        m_buffer << " ? ";
        node.then_branch().accept(*this);
        m_buffer << " : ";
        node.else_branch().accept(*this);
        m_buffer << ")";
    }

    void visit(const CallExprAST& node) override {
        node.callee().accept(*this);
        m_buffer << "(";
        for (size_t i = 0; i < node.arguments().size(); ++i) {
            node.arguments()[i]->accept(*this);
            if (i + 1 < node.arguments().size()) {
                m_buffer << ", ";
            }
        }
        m_buffer << ")";
    }

    void visit(const BlockExprAST& node) override {
        // Emit as an Immediately Invoked Lambda Expression (IILE)
        m_buffer << "([&]{\n";
        indent();

        for (const auto& stmt : node.statements()) {
            stmt->accept(*this);
        }

        if (node.result_expr()) {
            emit_indent();
            m_buffer << "return ";
            node.result_expr()->accept(*this);
            m_buffer << ";\n";
        }

        dedent();
        emit_indent();
        m_buffer << "})()";
    }

    void visit(const LambdaExprAST& node) override {
        m_buffer << "[&](this auto self_";
        for (size_t i = 0; i < node.parameters().size(); ++i) {
            m_buffer << ", " << map_type(node.parameters()[i].type)
                << " " << node.parameters()[i].name;
        }
        m_buffer << ")";

        if (node.return_type().has_value()) {
            m_buffer << " -> " << map_type(*node.return_type()) << " ";
        }

        m_buffer << "{return ";        
        node.body().accept(*this);
        m_buffer << ";}";
    }

private:
    std::ostringstream m_buffer;
    std::string m_self;
    bool m_has_main{false};
    int m_indent_level{0};

    void indent() noexcept { ++m_indent_level; }
    void dedent() noexcept { if (m_indent_level > 0) --m_indent_level; }

    void emit_indent() {
        for (int i = 0; i < m_indent_level; ++i) {
            m_buffer << "    ";
        }
    }

    void emit_line(std::string_view line) {
        emit_indent();
        m_buffer << line << "\n";
    }

    static std::string map_type(const TypeNode& type) {
        switch (type.kind()) {
            case TypeNode::Kind::Primitive:
                switch (type.primitive_kind()) {
                    case PrimitiveTypeKind::Int:    return "long long";
                    case PrimitiveTypeKind::Float:  return "double";
                    case PrimitiveTypeKind::String: return "std::string";
                    case PrimitiveTypeKind::Bool:   return "bool";
                    case PrimitiveTypeKind::Void:   return "void";
                }
                break;
            case TypeNode::Kind::Function:
                // Recursive mapping for higher-order function types
                return "Function";
            case TypeNode::Kind::List:
                return "std::vector<std::any>";
        }
        return "std::any";
    }

    static std::string_view map_operator(TokenType op) noexcept {
        switch (op) {
            case TokenType::Plus:               return "+";
            case TokenType::Minus:              return "-";
            case TokenType::Asterisk:           return "*";
            case TokenType::Slash:              return "/";
            case TokenType::Percent:            return "%";
            case TokenType::EqualEqual:         return "==";
            case TokenType::ExclamationEqual:   return "!=";
            case TokenType::Less:               return "<";
            case TokenType::LessEqual:          return "<=";
            case TokenType::Greater:            return ">";
            case TokenType::GreaterEqual:       return ">=";
            case TokenType::AmpersandAmpersand: return "&&";
            case TokenType::PipePipe:           return "||";
            default:                            return "/* ? */";
        }
    }

    static std::string escape_string(std::string_view raw) {
        std::string out;
        for (char c : raw) {
            if (c == '"') out += "\\\"";
            else if (c == '\\') out += "\\\\";
            else if (c == '\n') out += "\\n";
            else if (c == '\t') out += "\\t";
            else if (c == '\r') out += "\\r";
            else out += c;
        }
        return out;
    }
};

} // namespace Faberge

#endif // EMITTER_HPP