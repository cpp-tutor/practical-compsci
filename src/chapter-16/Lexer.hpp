#ifndef LEXER_HPP
#define LEXER_HPP

#include "CoreDB.hpp"

namespace CoreDB {

enum class TokenType {
    Keyword_CREATE, Keyword_TABLE, Keyword_INSERT, Keyword_INTO,
    Keyword_VALUES, Keyword_SELECT, Keyword_FROM, Keyword_WHERE,
    Keyword_PRIMARY, Keyword_KEY, Keyword_NOT, Keyword_NULL,
    Type_INT, Type_FLOAT, Type_TEXT,
    Identifier,
    Literal_Integer, Literal_Float, Literal_String,
    Comma, LeftParen, RightParen, Asterisk,
    Op_Equal, Op_NotEqual, Op_Less, Op_LessEqual, Op_Greater, Op_GreaterEqual,
    SemiColon,
    EndOfFile
};

struct Token {
    TokenType type;
    std::string text;
    size_t line{1};
};

class Lexer {
public:
    explicit Lexer(std::string source) : m_src(std::move(source)), m_pos(0) {}

    std::vector<Token> tokenise() {
        std::vector<Token> tokens;
        while (!is_eof()) {
            char c = peek();
            if (std::isspace(c)) {
                advance();
                continue;
            }
            if (std::isalpha(c) || c == '_') {
                tokens.push_back(lex_identifier());
                continue;
            }
            if (std::isdigit(c) || (c == '-' && std::isdigit(peek_next()))) {
                tokens.push_back(lex_number());
                continue;
            }
            if (c == '\'') {
                tokens.push_back(lex_string());
                continue;
            }
            tokens.push_back(lex_symbol());
        }
        tokens.push_back({TokenType::EndOfFile, "", m_line});
        return tokens;
    }

private:
    std::string m_src;
    size_t m_pos{0};
    size_t m_line{1};

    [[nodiscard]] bool is_eof() const noexcept { return m_pos >= m_src.size(); }
    [[nodiscard]] char peek() const noexcept { return is_eof() ? '\0' : m_src[m_pos]; }
    [[nodiscard]] char peek_next() const noexcept { 
        return (m_pos + 1 >= m_src.size()) ? '\0' : m_src[m_pos + 1]; 
    }
    char advance() noexcept { return m_src[m_pos++]; }

    Token lex_identifier() {
        size_t start = m_pos;
        while (!is_eof() && (std::isalnum(peek()) || peek() == '_')) {
            advance();
        }
        std::string raw = m_src.substr(start, m_pos - start);
        std::string upper = raw;
        std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);

        if (upper == "CREATE")   return {TokenType::Keyword_CREATE, raw, m_line};
        if (upper == "TABLE")    return {TokenType::Keyword_TABLE, raw, m_line};
        if (upper == "INSERT")   return {TokenType::Keyword_INSERT, raw, m_line};
        if (upper == "INTO")     return {TokenType::Keyword_INTO, raw, m_line};
        if (upper == "VALUES")   return {TokenType::Keyword_VALUES, raw, m_line};
        if (upper == "SELECT")   return {TokenType::Keyword_SELECT, raw, m_line};
        if (upper == "FROM")     return {TokenType::Keyword_FROM, raw, m_line};
        if (upper == "WHERE")    return {TokenType::Keyword_WHERE, raw, m_line};
        if (upper == "PRIMARY")  return {TokenType::Keyword_PRIMARY, raw, m_line};
        if (upper == "KEY")      return {TokenType::Keyword_KEY, raw, m_line};
        if (upper == "NOT")      return {TokenType::Keyword_NOT, raw, m_line};
        if (upper == "NULL")     return {TokenType::Keyword_NULL, raw, m_line};
        if (upper == "INT")      return {TokenType::Type_INT, raw, m_line};
        if (upper == "FLOAT")    return {TokenType::Type_FLOAT, raw, m_line};
        if (upper == "TEXT")     return {TokenType::Type_TEXT, raw, m_line};

        return {TokenType::Identifier, raw, m_line};
    }

    Token lex_number() {
        size_t start = m_pos;
        if (peek() == '-') advance();
        bool is_float = false;
        while (!is_eof() && std::isdigit(peek())) advance();
        if (!is_eof() && peek() == '.') {
            is_float = true;
            advance();
            while (!is_eof() && std::isdigit(peek())) advance();
        }
        std::string val = m_src.substr(start, m_pos - start);
        return {is_float ? TokenType::Literal_Float : TokenType::Literal_Integer, val, m_line};
    }

    Token lex_string() {
        advance(); // Skip opening '
        size_t start = m_pos;
        while (!is_eof() && peek() != '\'') {
            advance();
        }
        if (is_eof()) throw std::runtime_error("Lexer error: Unterminated string literal.");
        std::string val = m_src.substr(start, m_pos - start);
        advance(); // Skip closing '
        return {TokenType::Literal_String, val, m_line};
    }

    Token lex_symbol() {
        char c = advance();
        switch (c) {
            case '(': return {TokenType::LeftParen, "(", m_line};
            case ')': return {TokenType::RightParen, ")", m_line};
            case ',': return {TokenType::Comma, ",", m_line};
            case '*': return {TokenType::Asterisk, "*", m_line};
            case '=': return {TokenType::Op_Equal, "=", m_line};
            case '<':
                if (peek() == '=') { advance(); return {TokenType::Op_LessEqual, "<=", m_line}; }
                if (peek() == '>') { advance(); return {TokenType::Op_NotEqual, "<>", m_line}; }
                return {TokenType::Op_Less, "<", m_line};
            case '>':
                if (peek() == '=') { advance(); return {TokenType::Op_GreaterEqual, ">=", m_line}; }
                return {TokenType::Op_Greater, ">", m_line};
            case '!':
                if (peek() == '=') { advance(); return {TokenType::Op_NotEqual, "!=", m_line}; }
                break;
            case ';': return {TokenType::SemiColon, ";", m_line};
            default: break;
        }
        throw std::runtime_error(std::string("Lexer error: Unknown symbol '") + c + "'");
    }
};

} // namespace CoreDB

#endif // LEXER_HPP