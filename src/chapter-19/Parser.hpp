#ifndef PARSER_HPP
#define PARSER_HPP

#include "../chapter-18/Token.hpp"
#include "AST.hpp"
#include "../chapter-18/Diagnostic.hpp"
#include <vector>
#include <memory>
#include <stdexcept>
#include <span>

namespace Faberge {

class Parser {
public:
    explicit Parser(std::vector<Token> tokens, std::string_view source_code)
        : m_tokens(std::move(tokens)), m_source_code(source_code), m_cursor(0) {}

    std::unique_ptr<ProgramAST> parse_program() {
        std::vector<std::unique_ptr<StmtAST>> statements;
        SourceLocation start_loc = current_token().location;

        while (!is_at_end()) {
            try {
                statements.push_back(parse_statement());
            } catch (const std::runtime_error& e) {
                synchronise();
            }
        }

        return std::make_unique<ProgramAST>(start_loc, std::move(statements));
    }

    [[nodiscard]] bool has_errors() const noexcept { return m_error_count > 0; }
    [[nodiscard]] size_t error_count() const noexcept { return m_error_count; }

private:
    std::vector<Token> m_tokens;
    std::string_view m_source_code;
    size_t m_cursor{0};
    size_t m_error_count{0};

    // -------------------------------------------------------------------------
    // Token Navigation Helpers
    // -------------------------------------------------------------------------
    [[nodiscard]] bool is_at_end() const noexcept {
        return m_cursor >= m_tokens.size() || current_token().type == TokenType::EndOfFile;
    }

    [[nodiscard]] const Token& current_token() const noexcept {
        return m_tokens[m_cursor];
    }

    [[nodiscard]] const Token& previous_token() const noexcept {
        return m_tokens[m_cursor - 1];
    }

    const Token& advance() noexcept {
        if (!is_at_end()) ++m_cursor;
        return previous_token();
    }

    bool check(TokenType type) const noexcept {
        if (is_at_end()) return false;
        return current_token().type == type;
    }

    bool match(TokenType type) noexcept {
        if (check(type)) {
            advance();
            return true;
        }
        return false;
    }

    const Token& consume(TokenType expected, std::string_view err_msg) {
        if (check(expected)) return advance();
        
        ++m_error_count;
        DiagnosticEngine::report_error(m_source_code, current_token().location, err_msg);
        throw std::runtime_error(std::string(err_msg));
    }

    // -------------------------------------------------------------------------
    // Panic-Mode Error Recovery
    // -------------------------------------------------------------------------
    void synchronise() noexcept {
        advance();
        while (!is_at_end()) {
            if (previous_token().type == TokenType::Semicolon) return;

            switch (current_token().type) {
                case TokenType::Keyword_Let:
                case TokenType::Keyword_Fn:
                case TokenType::Keyword_If:
                case TokenType::Keyword_Return:
                    return;
                default:
                    advance();
                    break;
            }
        }
    }

    // -------------------------------------------------------------------------
    // Statement Parsing
    // -------------------------------------------------------------------------
    std::unique_ptr<StmtAST> parse_statement() {
        if (match(TokenType::Keyword_Let)) {
            return parse_let_statement();
        }
        return parse_expression_statement();
    }

    std::unique_ptr<StmtAST> parse_let_statement() {
        SourceLocation loc = previous_token().location;
        Token name_tok = consume(TokenType::Identifier, "Expected variable name after 'let'.");

        std::optional<TypeNode> type_annot;
        if (match(TokenType::Colon)) {
            type_annot = parse_type();
        }

        consume(TokenType::Equal, "Expected '=' in variable declaration.");
        auto init_expr = parse_expression();
        consume(TokenType::Semicolon, "Expected ';' after let statement.");

        return std::make_unique<LetStmtAST>(loc, std::string(name_tok.lexeme), 
                                            std::move(type_annot), std::move(init_expr));
    }

    std::unique_ptr<StmtAST> parse_expression_statement() {
        SourceLocation loc = current_token().location;
        auto expr = parse_expression();
        consume(TokenType::Semicolon, "Expected ';' after expression statement.");
        return std::make_unique<ExprStmtAST>(loc, std::move(expr));
    }

    TypeNode parse_type() {
        if (match(TokenType::Identifier)) {
            std::string_view name = previous_token().lexeme;
            if (name == "Int")    return TypeNode(PrimitiveTypeKind::Int);
            if (name == "Float")  return TypeNode(PrimitiveTypeKind::Float);
            if (name == "String") return TypeNode(PrimitiveTypeKind::String);
            if (name == "Bool")   return TypeNode(PrimitiveTypeKind::Bool);
            if (name == "Void")   return TypeNode(PrimitiveTypeKind::Void);
            throw std::runtime_error("Unknown type identifier '" + std::string(name) + "'.");
        }
        throw std::runtime_error("Expected type name.");
    }

    // -------------------------------------------------------------------------
    // Pratt Expression Parsing
    // -------------------------------------------------------------------------
    static int get_infix_precedence(TokenType type) noexcept {
        switch (type) {
            case TokenType::PipePipe:           return 10; // ||
            case TokenType::AmpersandAmpersand: return 20; // &&
            case TokenType::EqualEqual:
            case TokenType::ExclamationEqual:   return 30; // ==, !=
            case TokenType::Less:
            case TokenType::LessEqual:
            case TokenType::Greater:
            case TokenType::GreaterEqual:       return 40; // <, <=, >, >=
            case TokenType::Plus:
            case TokenType::Minus:              return 50; // +, -
            case TokenType::Asterisk:
            case TokenType::Slash:
            case TokenType::Percent:            return 60; // *, /, %
            case TokenType::LeftParen:          return 70; // Function Call: f(...)
            default:                            return 0;
        }
    }

    std::unique_ptr<ExprAST> parse_expression(int min_precedence = 0) {
        auto left = parse_prefix();

        while (min_precedence < get_infix_precedence(current_token().type)) {
            left = parse_infix(std::move(left));
        }

        return left;
    }

    std::unique_ptr<ExprAST> parse_prefix() {
        SourceLocation loc = current_token().location;

        // 1. Unary Operators: -, !
        if (match(TokenType::Minus) || match(TokenType::Exclamation)) {
            TokenType op = previous_token().type;
            auto operand = parse_expression(60); // High unary binding power
            return std::make_unique<UnaryExprAST>(loc, op, std::move(operand));
        }

        // 2. Literals
        if (match(TokenType::Literal_Integer) || 
            match(TokenType::Literal_Float)   || 
            match(TokenType::Literal_String)  || 
            match(TokenType::Literal_Boolean)) {
            return std::make_unique<LiteralExprAST>(loc, previous_token().type, previous_token().lexeme);
        }

        // 3. Variables and Identifiers
        if (match(TokenType::Identifier)) {
            return std::make_unique<VariableExprAST>(loc, std::string(previous_token().lexeme));
        }

        // 4. Parenthesised Grouping: ( expr )
        if (match(TokenType::LeftParen)) {
            auto inner = parse_expression(0);
            consume(TokenType::RightParen, "Expected ')' after grouping expression.");
            return inner;
        }

        // 5. Block Expressions: { stmt* expr? }
        if (match(TokenType::LeftBrace)) {
            return parse_block_expression();
        }

        // 6. Conditional Expressions: if cond then expr else expr
        if (match(TokenType::Keyword_If)) {
            return parse_if_expression();
        }

        // 7. Lambda Expressions: fn (params) => body
        if (match(TokenType::Keyword_Fn)) {
            return parse_lambda_expression();
        }

        ++m_error_count;
        DiagnosticEngine::report_error(m_source_code, current_token().location, 
                                       "Expected expression, found '" + std::string(current_token().lexeme) + "'.");
        throw std::runtime_error("Syntax error: unexpected expression token.");
    }

    std::unique_ptr<ExprAST> parse_infix(std::unique_ptr<ExprAST> left) {
        SourceLocation loc = current_token().location;
        Token op_tok = advance();

        // Function Call Operator: callee(arg1, arg2)
        if (op_tok.type == TokenType::LeftParen) {
            std::vector<std::unique_ptr<ExprAST>> args;
            if (!check(TokenType::RightParen)) {
                do {
                    args.push_back(parse_expression(0));
                } while (match(TokenType::Comma));
            }
            consume(TokenType::RightParen, "Expected ')' after argument list.");
            return std::make_unique<CallExprAST>(loc, std::move(left), std::move(args));
        }

        // Standard Binary Arithmetic and Logical Operators
        int prec = get_infix_precedence(op_tok.type);
        auto right = parse_expression(prec); // Left-associative recursion

        return std::make_unique<BinaryExprAST>(loc, op_tok.type, std::move(left), std::move(right));
    }

    std::unique_ptr<ExprAST> parse_block_expression() {
        SourceLocation loc = previous_token().location;
        std::vector<std::unique_ptr<StmtAST>> stmts;
        std::unique_ptr<ExprAST> result_expr;

        while (!check(TokenType::RightBrace) && !is_at_end()) {
            if (check(TokenType::Keyword_Let)) {
                stmts.push_back(parse_statement());
            } else {
                // Peek ahead to check if this is an expression-statement (with ';') or a tail expression
                auto expr = parse_expression();
                if (match(TokenType::Semicolon)) {
                    stmts.push_back(std::make_unique<ExprStmtAST>(expr->location(), std::move(expr)));
                } else {
                    result_expr = std::move(expr);
                    break;
                }
            }
        }

        consume(TokenType::RightBrace, "Expected '}' to close block expression.");
        return std::make_unique<BlockExprAST>(loc, std::move(stmts), std::move(result_expr));
    }

    std::unique_ptr<ExprAST> parse_if_expression() {
        SourceLocation loc = previous_token().location;
        auto cond = parse_expression();
        consume(TokenType::Keyword_Then, "Expected 'then' after if condition.");
        auto then_b = parse_expression();
        consume(TokenType::Keyword_Else, "Expected 'else' in conditional expression.");
        auto else_b = parse_expression();

        return std::make_unique<IfExprAST>(loc, std::move(cond), std::move(then_b), std::move(else_b));
    }

    std::unique_ptr<ExprAST> parse_lambda_expression() {
        SourceLocation loc = previous_token().location;
        consume(TokenType::LeftParen, "Expected '(' after 'fn'.");

        std::vector<Parameter> params;
        if (!check(TokenType::RightParen)) {
            do {
                Token param_name = consume(TokenType::Identifier, "Expected parameter name.");
                consume(TokenType::Colon, "Expected ':' after parameter name.");
                TypeNode param_type = parse_type();
                params.push_back({std::string(param_name.lexeme), std::move(param_type), param_name.location});
            } while (match(TokenType::Comma));
        }

        consume(TokenType::RightParen, "Expected ')' after parameter list.");

        std::optional<TypeNode> ret_type;
        if (match(TokenType::Colon)) {
            ret_type = parse_type();
        }

        consume(TokenType::FatArrow, "Expected '=>' in function definition.");
        auto body = parse_expression();

        return std::make_unique<LambdaExprAST>(loc, std::move(params), std::move(ret_type), std::move(body));
    }
};

} // namespace Faberge

#endif // PARSER_HPP