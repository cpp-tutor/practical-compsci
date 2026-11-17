#ifndef AST_VISTOR_HPP
#define AST_VISTOR_HPP

#include "AST.hpp"
#include <iostream>

namespace Faberge {

class ASTVisitor {
public:
    virtual ~ASTVisitor() = default;

    virtual void visit(const ProgramAST& node) = 0;
    virtual void visit(const LetStmtAST& node) = 0;
    virtual void visit(const ExprStmtAST& node) = 0;
    virtual void visit(const LiteralExprAST& node) = 0;
    virtual void visit(const VariableExprAST& node) = 0;
    virtual void visit(const UnaryExprAST& node) = 0;
    virtual void visit(const BinaryExprAST& node) = 0;
    virtual void visit(const IfExprAST& node) = 0;
    virtual void visit(const CallExprAST& node) = 0;
    virtual void visit(const BlockExprAST& node) = 0;
    virtual void visit(const LambdaExprAST& node) = 0;
};

// Inline definitions of double-dispatch accept methods
inline void ProgramAST::accept(ASTVisitor& v) const     { v.visit(*this); }
inline void LetStmtAST::accept(ASTVisitor& v) const     { v.visit(*this); }
inline void ExprStmtAST::accept(ASTVisitor& v) const    { v.visit(*this); }
inline void LiteralExprAST::accept(ASTVisitor& v) const { v.visit(*this); }
inline void VariableExprAST::accept(ASTVisitor& v) const{ v.visit(*this); }
inline void UnaryExprAST::accept(ASTVisitor& v) const   { v.visit(*this); }
inline void BinaryExprAST::accept(ASTVisitor& v) const  { v.visit(*this); }
inline void IfExprAST::accept(ASTVisitor& v) const      { v.visit(*this); }
inline void CallExprAST::accept(ASTVisitor& v) const    { v.visit(*this); }
inline void BlockExprAST::accept(ASTVisitor& v) const   { v.visit(*this); }
inline void LambdaExprAST::accept(ASTVisitor& v) const  { v.visit(*this); }

// Diagnostic AST Pretty-Printer
class ASTPrinter : public ASTVisitor {
public:
    void print(const ASTNode& node) {
        m_indent_level = 0;
        node.accept(*this);
    }

    void visit(const ProgramAST& node) override {
        print_indent();
        std::cout << "\033[1;36mProgram\033[0m\n";
        indent();
        for (const auto& stmt : node.statements()) {
            stmt->accept(*this);
        }
        dedent();
    }

    void visit(const LetStmtAST& node) override {
        print_indent();
        std::cout << "\033[1;32mLetStmt\033[0m (name: '\033[1m" << node.name() << "\033[0m'";
        if (node.type_annotation()) {
            std::cout << ", type: \033[33m" << node.type_annotation()->to_string() << "\033[0m";
        }
        std::cout << ")\n";
        indent();
        node.initialiser().accept(*this);
        dedent();
    }

    void visit(const ExprStmtAST& node) override {
        print_indent();
        std::cout << "\033[1;34mExprStmt\033[0m\n";
        indent();
        node.expression().accept(*this);
        dedent();
    }

    void visit(const LiteralExprAST& node) override {
        print_indent();
        std::cout << "\033[33mLiteral\033[0m (" << node.value() << ")\n";
    }

    void visit(const VariableExprAST& node) override {
        print_indent();
        std::cout << "\033[35mVariable\033[0m (" << node.name() << ")\n";
    }

    void visit(const UnaryExprAST& node) override {
        print_indent();
        std::cout << "\033[31mUnaryExpr\033[0m (op: " << Token::to_string(node.op()) << ")\n";
        indent();
        node.operand().accept(*this);
        dedent();
    }

    void visit(const BinaryExprAST& node) override {
        print_indent();
        std::cout << "\033[1;31mBinaryExpr\033[0m (op: " << Token::to_string(node.op()) << ")\n";
        indent();
        node.left().accept(*this);
        node.right().accept(*this);
        dedent();
    }

    void visit(const IfExprAST& node) override {
        print_indent();
        std::cout << "\033[1;35mIfExpr\033[0m\n";
        indent();
        node.condition().accept(*this);
        node.then_branch().accept(*this);
        node.else_branch().accept(*this);
        dedent();
    }

    void visit(const CallExprAST& node) override {
        print_indent();
        std::cout << "\033[1;34mCallExpr\033[0m\n";
        indent();
        node.callee().accept(*this);
        for (const auto& arg : node.arguments()) {
            arg->accept(*this);
        }
        dedent();
    }

    void visit(const BlockExprAST& node) override {
        print_indent();
        std::cout << "\033[1;36mBlockExpr\033[0m\n";
        indent();
        for (const auto& stmt : node.statements()) {
            stmt->accept(*this);
        }
        if (node.result_expr()) {
            node.result_expr()->accept(*this);
        }
        dedent();
    }

    void visit(const LambdaExprAST& node) override {
        print_indent();
        std::cout << "\033[1;32mLambdaExpr\033[0m (";
        for (size_t i = 0; i < node.parameters().size(); ++i) {
            std::cout << node.parameters()[i].name << ": " << node.parameters()[i].type.to_string();
            if (i + 1 < node.parameters().size()) std::cout << ", ";
        }
        std::cout << ")\n";
        indent();
        node.body().accept(*this);
        dedent();
    }

private:
    int m_indent_level{0};

    void indent() noexcept { ++m_indent_level; }
    void dedent() noexcept { --m_indent_level; }

    void print_indent() const {
        for (int i = 0; i < m_indent_level; ++i) {
            std::cout << "  ";
        }
    }
};

} // namespace Faberge

#endif // AST_VISITOR_HPP