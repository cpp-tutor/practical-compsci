#ifndef AST_HPP
#define AST_HPP

#include "Token.hpp"
#include <string>
#include <vector>
#include <memory>
#include <string_view>
#include <optional>

namespace Faberge {

// Forward declarations
class ASTVisitor;

// -----------------------------------------------------------------------------
// Base AST Node
// -----------------------------------------------------------------------------
class ASTNode {
public:
    explicit ASTNode(SourceLocation loc) : m_location(loc) {}
    virtual ~ASTNode() = default;

    [[nodiscard]] SourceLocation location() const noexcept { return m_location; }
    virtual void accept(ASTVisitor& visitor) const = 0;

private:
    SourceLocation m_location;
};

// -----------------------------------------------------------------------------
// Types Representation
// -----------------------------------------------------------------------------
enum class PrimitiveTypeKind {
    Int,
    Float,
    String,
    Bool,
    Void
};

class TypeNode {
public:
    enum class Kind { Primitive, Function, List };

    explicit TypeNode(PrimitiveTypeKind prim) 
        : m_kind(Kind::Primitive), m_primitive_kind(prim) {}

    static TypeNode make_function(TypeNode param, TypeNode ret) {
        TypeNode t(Kind::Function);
        t.m_param_types.push_back(std::make_shared<TypeNode>(std::move(param)));
        t.m_return_type = std::make_shared<TypeNode>(std::move(ret));
        return t;
    }

    [[nodiscard]] Kind kind() const noexcept { return m_kind; }
    [[nodiscard]] PrimitiveTypeKind primitive_kind() const noexcept { return m_primitive_kind; }

    [[nodiscard]] std::string to_string() const {
        switch (m_kind) {
            case Kind::Primitive:
                switch (m_primitive_kind) {
                    case PrimitiveTypeKind::Int:    return "Int";
                    case PrimitiveTypeKind::Float:  return "Float";
                    case PrimitiveTypeKind::String: return "String";
                    case PrimitiveTypeKind::Bool:   return "Bool";
                    case PrimitiveTypeKind::Void:   return "Void";
                }
                break;
            case Kind::Function:
                return "(" + (m_param_types.empty() ? "Void" : m_param_types[0]->to_string()) + 
                       " -> " + m_return_type->to_string() + ")";
            case Kind::List:
                return "[" + m_param_types[0]->to_string() + "]";
        }
        return "Unknown";
    }

private:
    explicit TypeNode(Kind k) : m_kind(k) {}
    Kind m_kind;
    PrimitiveTypeKind m_primitive_kind{PrimitiveTypeKind::Void};
    std::vector<std::shared_ptr<TypeNode>> m_param_types;
    std::shared_ptr<TypeNode> m_return_type;
};

// -----------------------------------------------------------------------------
// Expression & Statement Base Nodes (Defined before any concrete AST nodes)
// -----------------------------------------------------------------------------

class ExprAST : public ASTNode {
public:
    using ASTNode::ASTNode;
};

class StmtAST : public ASTNode {
public:
    using ASTNode::ASTNode;
};

// -----------------------------------------------------------------------------
// Expression Nodes
// -----------------------------------------------------------------------------

class LiteralExprAST : public ExprAST {
public:
    LiteralExprAST(SourceLocation loc, TokenType type, std::string_view value)
        : ExprAST(loc), m_token_type(type), m_value(value) {}

    [[nodiscard]] TokenType token_type() const noexcept { return m_token_type; }
    [[nodiscard]] std::string_view value() const noexcept { return m_value; }

    void accept(ASTVisitor& visitor) const override;

private:
    TokenType m_token_type;
    std::string m_value;
};

class VariableExprAST : public ExprAST {
public:
    VariableExprAST(SourceLocation loc, std::string name)
        : ExprAST(loc), m_name(std::move(name)) {}

    [[nodiscard]] const std::string& name() const noexcept { return m_name; }

    void accept(ASTVisitor& visitor) const override;

private:
    std::string m_name;
};

class UnaryExprAST : public ExprAST {
public:
    UnaryExprAST(SourceLocation loc, TokenType op, std::unique_ptr<ExprAST> operand)
        : ExprAST(loc), m_op(op), m_operand(std::move(operand)) {}

    [[nodiscard]] TokenType op() const noexcept { return m_op; }
    [[nodiscard]] const ExprAST& operand() const noexcept { return *m_operand; }

    void accept(ASTVisitor& visitor) const override;

private:
    TokenType m_op;
    std::unique_ptr<ExprAST> m_operand;
};

class BinaryExprAST : public ExprAST {
public:
    BinaryExprAST(SourceLocation loc, TokenType op, 
                   std::unique_ptr<ExprAST> left, 
                   std::unique_ptr<ExprAST> right)
        : ExprAST(loc), m_op(op), m_left(std::move(left)), m_right(std::move(right)) {}

    [[nodiscard]] TokenType op() const noexcept { return m_op; }
    [[nodiscard]] const ExprAST& left() const noexcept { return *m_left; }
    [[nodiscard]] const ExprAST& right() const noexcept { return *m_right; }

    void accept(ASTVisitor& visitor) const override;

private:
    TokenType m_op;
    std::unique_ptr<ExprAST> m_left;
    std::unique_ptr<ExprAST> m_right;
};

class IfExprAST : public ExprAST {
public:
    IfExprAST(SourceLocation loc, 
               std::unique_ptr<ExprAST> condition, 
               std::unique_ptr<ExprAST> then_branch, 
               std::unique_ptr<ExprAST> else_branch)
        : ExprAST(loc), m_condition(std::move(condition)), 
          m_then_branch(std::move(then_branch)), m_else_branch(std::move(else_branch)) {}

    [[nodiscard]] const ExprAST& condition() const noexcept { return *m_condition; }
    [[nodiscard]] const ExprAST& then_branch() const noexcept { return *m_then_branch; }
    [[nodiscard]] const ExprAST& else_branch() const noexcept { return *m_else_branch; }

    void accept(ASTVisitor& visitor) const override;

private:
    std::unique_ptr<ExprAST> m_condition;
    std::unique_ptr<ExprAST> m_then_branch;
    std::unique_ptr<ExprAST> m_else_branch;
};

class CallExprAST : public ExprAST {
public:
    CallExprAST(SourceLocation loc, std::unique_ptr<ExprAST> callee, 
                 std::vector<std::unique_ptr<ExprAST>> args)
        : ExprAST(loc), m_callee(std::move(callee)), m_arguments(std::move(args)) {}

    [[nodiscard]] const ExprAST& callee() const noexcept { return *m_callee; }
    [[nodiscard]] const std::vector<std::unique_ptr<ExprAST>>& arguments() const noexcept { 
        return m_arguments; 
    }

    void accept(ASTVisitor& visitor) const override;

private:
    std::unique_ptr<ExprAST> m_callee;
    std::vector<std::unique_ptr<ExprAST>> m_arguments;
};

class BlockExprAST : public ExprAST {
public:
    BlockExprAST(SourceLocation loc, 
                 std::vector<std::unique_ptr<StmtAST>> statements,
                 std::unique_ptr<ExprAST> result_expr) : ExprAST(loc), m_statements(std::move(statements)), m_result_expr(std::move(result_expr)) {}
    [[nodiscard]] const std::vector<std::unique_ptr<StmtAST>>& statements() const noexcept { 
        return m_statements; 
    }
    [[nodiscard]] const ExprAST* result_expr() const noexcept { return m_result_expr.get(); }

    void accept(ASTVisitor& visitor) const override;

private:
    std::vector<std::unique_ptr<StmtAST>> m_statements;
    std::unique_ptr<ExprAST> m_result_expr;
};

struct Parameter {
    std::string name;
    TypeNode type;
    SourceLocation location;
};

class LambdaExprAST : public ExprAST {
public:
    LambdaExprAST(SourceLocation loc, std::vector<Parameter> params, 
                  std::optional<TypeNode> return_type, 
                  std::unique_ptr<ExprAST> body)
        : ExprAST(loc), m_parameters(std::move(params)), 
          m_return_type(std::move(return_type)), m_body(std::move(body)) {}

    [[nodiscard]] const std::vector<Parameter>& parameters() const noexcept { return m_parameters; }
    [[nodiscard]] const std::optional<TypeNode>& return_type() const noexcept { return m_return_type; }
    [[nodiscard]] const ExprAST& body() const noexcept { return *m_body; }

    void accept(ASTVisitor& visitor) const override;

private:
    std::vector<Parameter> m_parameters;
    std::optional<TypeNode> m_return_type;
    std::unique_ptr<ExprAST> m_body;
};

// -----------------------------------------------------------------------------
// Statement Nodes
// -----------------------------------------------------------------------------

class LetStmtAST : public StmtAST {
public:
    LetStmtAST(SourceLocation loc, std::string name, 
               std::optional<TypeNode> type_annot, 
               std::unique_ptr<ExprAST> initialiser)
        : StmtAST(loc), m_name(std::move(name)), 
          m_type_annot(std::move(type_annot)), m_initialiser(std::move(initialiser)) {}

    [[nodiscard]] const std::string& name() const noexcept { return m_name; }
    [[nodiscard]] const std::optional<TypeNode>& type_annotation() const noexcept { return m_type_annot; }
    [[nodiscard]] const ExprAST& initialiser() const noexcept { return *m_initialiser; }

    void accept(ASTVisitor& visitor) const override;

private:
    std::string m_name;
    std::optional<TypeNode> m_type_annot;
    std::unique_ptr<ExprAST> m_initialiser;
};

class ExprStmtAST : public StmtAST {
public:
    ExprStmtAST(SourceLocation loc, std::unique_ptr<ExprAST> expr)
        : StmtAST(loc), m_expression(std::move(expr)) {}

    [[nodiscard]] const ExprAST& expression() const noexcept { return *m_expression; }

    void accept(ASTVisitor& visitor) const override;

private:
    std::unique_ptr<ExprAST> m_expression;
};

// -----------------------------------------------------------------------------
// Program Root Node
// -----------------------------------------------------------------------------
class ProgramAST : public ASTNode {
public:
    ProgramAST(SourceLocation loc, std::vector<std::unique_ptr<StmtAST>> stmts)
        : ASTNode(loc), m_statements(std::move(stmts)) {}

    [[nodiscard]] const std::vector<std::unique_ptr<StmtAST>>& statements() const noexcept { 
        return m_statements; 
    }

    void accept(ASTVisitor& visitor) const override;

private:
    std::vector<std::unique_ptr<StmtAST>> m_statements;
};

} // namespace Faberge

#endif // AST_HPP