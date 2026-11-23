#ifndef TOKEN_HPP
#define TOKEN_HPP

#include "SourceLocation.hpp"
#include <string_view>
#include <string>

namespace Faberge {

enum class TokenType {
    // End of Input
    EndOfFile,

    // Identifiers and Literals
    Identifier,
    Literal_Integer,
    Literal_Float,
    Literal_String,
    Literal_Boolean,

    // Keywords
    Keyword_Let,
    Keyword_Mut,
    Keyword_Fn,
    Keyword_If,
    Keyword_Then,
    Keyword_Else,
    Keyword_Match,
    Keyword_With,
    Keyword_Type,
    Keyword_Return,

    // Arithmetic Operators
    Plus,           // +
    Minus,          // -
    Asterisk,       // *
    Slash,          // /
    Percent,        // %

    // Comparison Operators
    EqualEqual,     // ==
    ExclamationEqual,// !=
    Less,           // <
    LessEqual,      // <=
    Greater,        // >
    GreaterEqual,   // >=

    // Logical Operators
    AmpersandAmpersand, // &&
    PipePipe,           // ||
    Exclamation,        // !

    // Assignment and Structural Symbols
    Equal,          // =
    FatArrow,       // =>
    ThinArrow,      // ->
    ColonColon,     // ::
    Pipe,           // |
    Colon,          // :
    Semicolon,      // ;
    Comma,          // ,
    Dot,            // .

    // Delimiters
    LeftParen,      // (
    RightParen,     // )
    LeftBrace,      // {
    RightBrace,     // }
    LeftBracket,    // [
    RightBracket,   // ]

    // Lexical Error Sentinel
    Error
};

struct Token {
    TokenType type{TokenType::EndOfFile};
    std::string_view lexeme{};
    SourceLocation location{};

    [[nodiscard]] bool is(TokenType expected) const noexcept { return type == expected; }
    [[nodiscard]] bool is_not(TokenType expected) const noexcept { return type != expected; }

    [[nodiscard]] static std::string_view to_string(TokenType t) noexcept {
        switch (t) {
            case TokenType::EndOfFile:          return "EOF";
            case TokenType::Identifier:         return "Identifier";
            case TokenType::Literal_Integer:    return "Literal_Integer";
            case TokenType::Literal_Float:      return "Literal_Float";
            case TokenType::Literal_String:     return "Literal_String";
            case TokenType::Literal_Boolean:    return "Literal_Boolean";
            case TokenType::Keyword_Let:        return "let";
            case TokenType::Keyword_Mut:        return "mut";
            case TokenType::Keyword_Fn:         return "fn";
            case TokenType::Keyword_If:         return "if";
            case TokenType::Keyword_Then:       return "then";
            case TokenType::Keyword_Else:       return "else";
            case TokenType::Keyword_Match:      return "match";
            case TokenType::Keyword_With:       return "with";
            case TokenType::Keyword_Type:       return "type";
            case TokenType::Keyword_Return:     return "return";
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
            case TokenType::Exclamation:        return "!";
            case TokenType::Equal:              return "=";
            case TokenType::FatArrow:           return "=>";
            case TokenType::ThinArrow:          return "->";
            case TokenType::ColonColon:         return "::";
            case TokenType::Pipe:               return "|";
            case TokenType::Colon:              return ":";
            case TokenType::Semicolon:          return ";";
            case TokenType::Comma:              return ",";
            case TokenType::Dot:                return ".";
            case TokenType::LeftParen:          return "(";
            case TokenType::RightParen:         return ")";
            case TokenType::LeftBrace:          return "{";
            case TokenType::RightBrace:         return "}";
            case TokenType::LeftBracket:        return "[";
            case TokenType::RightBracket:       return "]";
            case TokenType::Error:              return "Error";
        }
        return "Unknown";
    }
};

} // namespace Faberge

#endif // TOKEN_HPP