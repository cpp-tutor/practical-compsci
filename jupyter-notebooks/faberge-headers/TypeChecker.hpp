#ifndef TYPE_CHECKER_HPP
#define TYPE_CHECKER_HPP

#include "AST.hpp"
#include "ASTVisitor.hpp"
#include "SymbolTable.hpp"
#include "Diagnostic.hpp"
#include <unordered_map>

namespace Faberge {

class TypeChecker : public ASTVisitor {
public:
    explicit TypeChecker(std::string_view source_code)
        : m_source_code(source_code), 
          m_current_env(Environment::create(nullptr)) {}

    bool check(const ProgramAST& program) {
        m_error_count = 0;
        program.accept(*this);
        return m_error_count == 0;
    }

    [[nodiscard]] size_t error_count() const noexcept { return m_error_count; }

    [[nodiscard]] Type get_node_type(const ExprAST& expr) const {
        auto it = m_node_types.find(&expr);
        if (it != m_node_types.end()) return it->second;
        return Type::make_error();
    }

    // -------------------------------------------------------------------------
    // Visitor Dispatch Implementations
    // -------------------------------------------------------------------------

    void visit(const ProgramAST& node) override {
        for (const auto& stmt : node.statements()) {
            stmt->accept(*this);
        }
    }

    void visit(const LetStmtAST& node) override {
        // Pre-bind function identifier in environment to support recursive self-references
        const auto* lambda = dynamic_cast<const LambdaExprAST*>(&node.initialiser());
        bool prebound = false;
        Type prebound_type = Type::make_error();

        if (node.type_annotation().has_value()) {
            prebound_type = convert_ast_type(*node.type_annotation());
            prebound = true;
        } else if (lambda && lambda->return_type().has_value()) {
            std::vector<Type> param_types;
            for (const auto& p : lambda->parameters()) {
                param_types.push_back(convert_ast_type(p.type));
            }
            Type ret_type = convert_ast_type(*lambda->return_type());
            prebound_type = Type::make_function(std::move(param_types), ret_type);
            prebound = true;
        }

        if (prebound) {
            if (!m_current_env->define(node.name(), prebound_type, node.location(), lambda != nullptr)) {
                report_type_error(node.location(), 
                                  "Redeclaration of identifier '" + node.name() + "' in the same scope.");
            }
        }

        node.initialiser().accept(*this);
        Type init_type = get_node_type(node.initialiser());

        Type var_type = init_type;

        // If explicit type annotation exists, verify compatibility
        if (node.type_annotation().has_value()) {
            Type declared_type = convert_ast_type(*node.type_annotation());
            if (init_type != declared_type && !init_type.is_error()) {
                report_type_error(node.initialiser().location(),
                                  "Type mismatch in variable declaration: expected '" + 
                                  declared_type.to_string() + "', got '" + 
                                  init_type.to_string() + "'.");
            }
            var_type = declared_type;
        }

        if (!prebound) {
            if (!m_current_env->define(node.name(), var_type, node.location())) {
                report_type_error(node.location(), 
                                  "Redeclaration of identifier '" + node.name() + "' in the same scope.");
            }
        }
    }

    void visit(const ExprStmtAST& node) override {
        node.expression().accept(*this);
    }

    void visit(const LiteralExprAST& node) override {
        Type t = Type::make_error();
        switch (node.token_type()) {
            case TokenType::Literal_Integer: t = Type::make_int(); break;
            case TokenType::Literal_Float:   t = Type::make_float(); break;
            case TokenType::Literal_String:  t = Type::make_string(); break;
            case TokenType::Literal_Boolean: t = Type::make_bool(); break;
            default: break;
        }
        set_node_type(node, t);
    }

    void visit(const VariableExprAST& node) override {
        auto sym = m_current_env->lookup(node.name());
        if (sym == std::nullopt) {
            report_type_error(node.location(), 
                              "Use of undeclared identifier '" + node.name() + "'.");
            set_node_type(node, Type::make_error());
            return;
        }
        set_node_type(node, sym->type);
    }

    void visit(const UnaryExprAST& node) override {
        node.operand().accept(*this);
        Type operand_type = get_node_type(node.operand());

        if (node.op() == TokenType::Minus) {
            if (!operand_type.is_numeric() && !operand_type.is_error()) {
                report_type_error(node.location(), 
                                  "Unary '-' operator requires numeric operand, found '" + 
                                  operand_type.to_string() + "'.");
                set_node_type(node, Type::make_error());
                return;
            }
            set_node_type(node, operand_type);
            return;
        }

        if (node.op() == TokenType::Exclamation) {
            if (operand_type != Type::make_bool() && !operand_type.is_error()) {
                report_type_error(node.location(), 
                                  "Logical '!' operator requires Bool operand, found '" + 
                                  operand_type.to_string() + "'.");
                set_node_type(node, Type::make_error());
                return;
            }
            set_node_type(node, Type::make_bool());
            return;
        }

        set_node_type(node, Type::make_error());
    }

    void visit(const BinaryExprAST& node) override {
        node.left().accept(*this);
        node.right().accept(*this);

        Type left_t = get_node_type(node.left());
        Type right_t = get_node_type(node.right());

        if (left_t.is_error() || right_t.is_error()) {
            set_node_type(node, Type::make_error());
            return;
        }

        // 1. Arithmetic Operators (+, -, *, /, %)
        if (node.op() == TokenType::Plus || node.op() == TokenType::Minus ||
            node.op() == TokenType::Asterisk || node.op() == TokenType::Slash || 
            node.op() == TokenType::Percent) {
            
            // Allow string concatenation on '+'
            if (node.op() == TokenType::Plus && left_t == Type::make_string() && right_t == Type::make_string()) {
                set_node_type(node, Type::make_string());
                return;
            }

            if (!left_t.is_numeric() || !right_t.is_numeric() || left_t != right_t) {
                report_type_error(node.location(), 
                                  "Binary operator '" + std::string(Token::to_string(node.op())) + 
                                  "' expects matching numeric operands, found '" + 
                                  left_t.to_string() + "' and '" + right_t.to_string() + "'.");
                set_node_type(node, Type::make_error());
                return;
            }

            set_node_type(node, left_t);
            return;
        }

        // 2. Comparison Operators (==, !=, <, <=, >, >=)
        if (node.op() == TokenType::EqualEqual || node.op() == TokenType::ExclamationEqual) {
            if (left_t != right_t) {
                report_type_error(node.location(), 
                                  "Equality operator requires identical operand types, found '" + 
                                  left_t.to_string() + "' and '" + right_t.to_string() + "'.");
            }
            set_node_type(node, Type::make_bool());
            return;
        }

        if (node.op() == TokenType::Less || node.op() == TokenType::LessEqual ||
            node.op() == TokenType::Greater || node.op() == TokenType::GreaterEqual) {
            if (!left_t.is_numeric() || !right_t.is_numeric() || left_t != right_t) {
                report_type_error(node.location(), 
                                  "Relational comparison requires matching numeric types, found '" + 
                                  left_t.to_string() + "' and '" + right_t.to_string() + "'.");
            }
            set_node_type(node, Type::make_bool());
            return;
        }

        // 3. Logical Operators (&&, ||)
        if (node.op() == TokenType::AmpersandAmpersand || node.op() == TokenType::PipePipe) {
            if (left_t != Type::make_bool() || right_t != Type::make_bool()) {
                report_type_error(node.location(), 
                                  "Logical operator requires Bool operands, found '" + 
                                  left_t.to_string() + "' and '" + right_t.to_string() + "'.");
            }
            set_node_type(node, Type::make_bool());
            return;
        }

        set_node_type(node, Type::make_error());
    }

    void visit(const IfExprAST& node) override {
        node.condition().accept(*this);
        Type cond_t = get_node_type(node.condition());

        if (cond_t != Type::make_bool() && !cond_t.is_error()) {
            report_type_error(node.condition().location(), 
                              "If condition must evaluate to Bool, found '" + cond_t.to_string() + "'.");
        }

        node.then_branch().accept(*this);
        node.else_branch().accept(*this);

        Type then_t = get_node_type(node.then_branch());
        Type else_t = get_node_type(node.else_branch());

        if (then_t != else_t && !then_t.is_error() && !else_t.is_error()) {
            report_type_error(node.location(), 
                              "Branches of 'if' expression must have matching types. Then branch is '" + 
                              then_t.to_string() + "', Else branch is '" + else_t.to_string() + "'.");
            set_node_type(node, Type::make_error());
            return;
        }

        set_node_type(node, then_t);
    }

    void visit(const CallExprAST& node) override {
        node.callee().accept(*this);
        Type callee_t = get_node_type(node.callee());

        if (callee_t.is_error()) {
            set_node_type(node, Type::make_error());
            return;
        }

        if (callee_t.kind() != Type::Kind::Function) {
            report_type_error(node.location(), 
                              "Attempted to call non-function expression of type '" + 
                              callee_t.to_string() + "'.");
            set_node_type(node, Type::make_error());
            return;
        }

        if (node.arguments().size() != callee_t.param_types().size()) {
            report_type_error(node.location(), 
                              "Function expects " + std::to_string(callee_t.param_types().size()) + 
                              " arguments, but received " + std::to_string(node.arguments().size()) + ".");
            set_node_type(node, callee_t.return_type());
            return;
        }

        for (size_t i = 0; i < node.arguments().size(); ++i) {
            node.arguments()[i]->accept(*this);
            Type arg_t = get_node_type(*node.arguments()[i]);
            Type expected_param_t = callee_t.param_types()[i];

            if (arg_t != expected_param_t && !arg_t.is_error()) {
                report_type_error(node.arguments()[i]->location(), 
                                  "Argument " + std::to_string(i + 1) + " type mismatch: expected '" + 
                                  expected_param_t.to_string() + "', got '" + arg_t.to_string() + "'.");
            }
        }

        set_node_type(node, callee_t.return_type());
    }

    void visit(const BlockExprAST& node) override {
        push_scope();

        for (const auto& stmt : node.statements()) {
            stmt->accept(*this);
        }

        Type block_result_t = Type::make_void();
        if (node.result_expr()) {
            node.result_expr()->accept(*this);
            block_result_t = get_node_type(*node.result_expr());
        }

        pop_scope();
        set_node_type(node, block_result_t);
    }

    void visit(const LambdaExprAST& node) override {
        std::vector<Type> param_types;
        
        // Construct child environment for function parameters
        auto fn_env = Environment::create(m_current_env);

        for (const auto& p : node.parameters()) {
            Type pt = convert_ast_type(p.type);
            param_types.push_back(pt);
            fn_env->define(p.name, pt, p.location);
        }

        // Switch to function environment to evaluate body
        auto prev_env = m_current_env;
        m_current_env = fn_env;

        node.body().accept(*this);
        Type body_t = get_node_type(node.body());

        m_current_env = prev_env;

        Type return_t = body_t;
        if (node.return_type().has_value()) {
            Type declared_ret = convert_ast_type(*node.return_type());
            if (body_t != declared_ret && !body_t.is_error()) {
                report_type_error(node.body().location(), 
                                  "Function body type mismatch: expected return type '" + 
                                  declared_ret.to_string() + "', but body evaluates to '" + 
                                  body_t.to_string() + "'.");
            }
            return_t = declared_ret;
        }

        Type fn_type = Type::make_function(std::move(param_types), return_t);
        set_node_type(node, fn_type);
    }

private:
    std::string_view m_source_code;
    std::shared_ptr<Environment> m_current_env;
    std::unordered_map<const ExprAST*, Type> m_node_types;
    size_t m_error_count{0};

    void push_scope() {
        m_current_env = Environment::create(m_current_env);
    }

    void pop_scope() {
        if (m_current_env->parent()) {
            m_current_env = m_current_env->parent();
        }
    }

    void set_node_type(const ExprAST& expr, Type t) {
        m_node_types[&expr] = std::move(t);
    }

    void report_type_error(SourceLocation loc, std::string_view msg) {
        ++m_error_count;
        DiagnosticEngine::report_error(m_source_code, loc, msg);
    }

    static Type convert_ast_type(const TypeNode& ast_type) {
        switch (ast_type.kind()) {
            case TypeNode::Kind::Primitive:
                switch (ast_type.primitive_kind()) {
                    case PrimitiveTypeKind::Int:    return Type::make_int();
                    case PrimitiveTypeKind::Float:  return Type::make_float();
                    case PrimitiveTypeKind::String: return Type::make_string();
                    case PrimitiveTypeKind::Bool:   return Type::make_bool();
                    case PrimitiveTypeKind::Void:   return Type::make_void();
                }
                break;
            case TypeNode::Kind::Function:
                // Extendable for higher-order type annotations
                return Type::make_error();
            case TypeNode::Kind::List:
                return Type::make_error();
        }
        return Type::make_error();
    }
};

} // namespace Faberge

#endif // TYPE_CHECKER_HPP