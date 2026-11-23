#ifndef LEXER_HPP
#define LEXER_HPP

#include "Token.hpp"
#include "Diagnostic.hpp"
#include <unordered_map>
#include <vector>
#include <cctype>

namespace Faberge {

class Lexer {
public:
    explicit Lexer(std::string_view source, std::string_view filename = "<anonymous>")
        : m_source(source), m_filename(filename) {
        initialise_keywords();
    }

    std::vector<Token> tokenise() {
        std::vector<Token> tokens;

        while (!is_at_end()) {
            m_start_cursor = m_current_cursor;
            m_start_column = m_column;

            Token token = scan_token();
            if (token.type != TokenType::Error) {
                tokens.push_back(token);
            }
        }

        tokens.push_back(Token{
            .type = TokenType::EndOfFile,
            .lexeme = "",
            .location = current_location()
        });

        return tokens;
    }

    [[nodiscard]] bool has_errors() const noexcept { return m_error_count > 0; }
    [[nodiscard]] size_t error_count() const noexcept { return m_error_count; }

private:
    std::string_view m_source;
    std::string_view m_filename;
    size_t m_start_cursor{0};
    size_t m_current_cursor{0};
    uint32_t m_line{1};
    uint32_t m_column{1};
    uint32_t m_start_column{1};
    size_t m_error_count{0};

    std::unordered_map<std::string_view, TokenType> m_keywords;

    void initialise_keywords() {
        m_keywords["let"]    = TokenType::Keyword_Let;
        m_keywords["mut"]    = TokenType::Keyword_Mut;
        m_keywords["fn"]     = TokenType::Keyword_Fn;
        m_keywords["if"]     = TokenType::Keyword_If;
        m_keywords["then"]   = TokenType::Keyword_Then;
        m_keywords["else"]   = TokenType::Keyword_Else;
        m_keywords["match"]  = TokenType::Keyword_Match;
        m_keywords["with"]   = TokenType::Keyword_With;
        m_keywords["type"]   = TokenType::Keyword_Type;
        m_keywords["return"] = TokenType::Keyword_Return;
        m_keywords["true"]   = TokenType::Literal_Boolean;
        m_keywords["false"]  = TokenType::Literal_Boolean;
    }

    [[nodiscard]] bool is_at_end() const noexcept {
        return m_current_cursor >= m_source.size();
    }

    char advance() noexcept {
        if (is_at_end()) return '\0';
        char c = m_source[m_current_cursor++];
        ++m_column;
        return c;
    }

    [[nodiscard]] char peek() const noexcept {
        return is_at_end() ? '\0' : m_source[m_current_cursor];
    }

    [[nodiscard]] char peek_next() const noexcept {
        return (m_current_cursor + 1 >= m_source.size()) ? '\0' : m_source[m_current_cursor + 1];
    }

    bool match(char expected) noexcept {
        if (is_at_end() || m_source[m_current_cursor] != expected) {
            return false;
        }
        ++m_current_cursor;
        ++m_column;
        return true;
    }

    [[nodiscard]] SourceLocation current_location() const noexcept {
        return SourceLocation{
            .filename = m_filename,
            .line = m_line,
            .column = m_start_column,
            .offset = static_cast<uint32_t>(m_start_cursor)
        };
    }

    [[nodiscard]] Token make_token(TokenType type) const noexcept {
        return Token{
            .type = type,
            .lexeme = m_source.substr(m_start_cursor, m_current_cursor - m_start_cursor),
            .location = current_location()
        };
    }

    Token error_token(std::string_view message) {
        ++m_error_count;
        DiagnosticEngine::report_error(m_source, current_location(), message);
        return Token{
            .type = TokenType::Error,
            .lexeme = m_source.substr(m_start_cursor, m_current_cursor - m_start_cursor),
            .location = current_location()
        };
    }

    // -------------------------------------------------------------------------
    // Scanning Subroutines
    // -------------------------------------------------------------------------

    Token scan_token() {
        skip_trivia();
        if (is_at_end()) return make_token(TokenType::EndOfFile);

        m_start_cursor = m_current_cursor;
        m_start_column = m_column;

        char c = advance();

        // 1. Identifiers and Keywords
        if (std::isalpha(c) || c == '_') {
            return scan_identifier();
        }

        // 2. Numeric Literals (Integers, Hex, Floats)
        if (std::isdigit(c)) {
            return scan_number();
        }

        // 3. String Literals
        if (c == '"') {
            return scan_string();
        }

        // 4. Operators and Punctuation
        switch (c) {
            // Single-character delimiters
            case '(': return make_token(TokenType::LeftParen);
            case ')': return make_token(TokenType::RightParen);
            case '{': return make_token(TokenType::LeftBrace);
            case '}': return make_token(TokenType::RightBrace);
            case '[': return make_token(TokenType::LeftBracket);
            case ']': return make_token(TokenType::RightBracket);
            case ',': return make_token(TokenType::Comma);
            case ';': return make_token(TokenType::Semicolon);
            case '.': return make_token(TokenType::Dot);
            case '%': return make_token(TokenType::Percent);

            // Arithmetic and Structural Operators
            case '+': return make_token(TokenType::Plus);
            case '*': return make_token(TokenType::Asterisk);
            case '/': return make_token(TokenType::Slash);

            case '-': 
                return match('>') ? make_token(TokenType::ThinArrow) : make_token(TokenType::Minus);

            case '=':
                if (match('=')) return make_token(TokenType::EqualEqual);
                if (match('>')) return make_token(TokenType::FatArrow);
                return make_token(TokenType::Equal);

            case '!':
                return match('=') ? make_token(TokenType::ExclamationEqual) : make_token(TokenType::Exclamation);

            case '<':
                return match('=') ? make_token(TokenType::LessEqual) : make_token(TokenType::Less);

            case '>':
                return match('=') ? make_token(TokenType::GreaterEqual) : make_token(TokenType::Greater);

            case ':':
                return match(':') ? make_token(TokenType::ColonColon) : make_token(TokenType::Colon);

            case '|':
                return match('|') ? make_token(TokenType::PipePipe) : make_token(TokenType::Pipe);

            case '&':
                if (match('&')) return make_token(TokenType::AmpersandAmpersand);
                return error_token("Unexpected character '&'. Did you mean '&&'?");

            default:
                break;
        }

        return error_token(std::string("Unrecognised or illegal character '") + c + "'.");
    }

    Token scan_identifier() {
        while (std::isalnum(peek()) || peek() == '_') {
            advance();
        }

        std::string_view text = m_source.substr(m_start_cursor, m_current_cursor - m_start_cursor);
        
        // Keyword disambiguation
        auto it = m_keywords.find(text);
        if (it != m_keywords.end()) {
            return make_token(it->second);
        }

        return make_token(TokenType::Identifier);
    }

    Token scan_number() {
        bool is_float = false;

        // Check for Hexadecimal: 0x...
        if (m_source[m_start_cursor] == '0' && (peek() == 'x' || peek() == 'X')) {
            advance(); // consume 'x'
            if (!std::isxdigit(peek())) {
                return error_token("Hexadecimal literal requires at least one valid hex digit [0-9a-fA-F].");
            }
            while (std::isxdigit(peek())) {
                advance();
            }
            return make_token(TokenType::Literal_Integer);
        }

        // Standard Decimal Integer
        while (std::isdigit(peek())) {
            advance();
        }

        // Check for Fractional Float: .[0-9]+
        if (peek() == '.' && std::isdigit(peek_next())) {
            is_float = true;
            advance(); // Consume '.'
            while (std::isdigit(peek())) {
                advance();
            }
        }

        return make_token(is_float ? TokenType::Literal_Float : TokenType::Literal_Integer);
    }

    Token scan_string() {
        while (peek() != '"' && !is_at_end()) {
            if (peek() == '\n') {
                ++m_line;
                m_column = 1;
            }
            // Handle escape characters
            if (peek() == '\\') {
                advance(); // consume '\'
                if (is_at_end()) break;
                char esc = advance();
                if (esc != '"' && esc != '\\' && esc != 'n' && esc != 't' && esc != 'r') {
                    while (peek() != '"' && !is_at_end()) { // Get to closing '"'...
                        advance();
                    }
                    advance(); // ...and consume it
                    return error_token(std::string("Invalid escape sequence '\\") + esc + "' in string literal.");
                }
                continue;
            }
            advance();
        }

        if (is_at_end()) {
            return error_token("Unterminated string literal: missing closing '\"'.");
        }

        advance(); // Consume closing '"'
        return make_token(TokenType::Literal_String);
    }

    void skip_trivia() {
        while (!is_at_end()) {
            char c = peek();
            switch (c) {
                case ' ':
                case '\t':
                case '\r':
                    advance();
                    break;
                case '\n':
                    ++m_line;
                    m_column = 0; // Will be set to 1 on advance
                    advance();
                    break;

                case '/':
                    // Single-line comment: // ...
                    if (peek_next() == '/') {
                        while (peek() != '\n' && !is_at_end()) {
                            advance();
                        }
                    } 
                    // Multi-line block comment: /* ... */ (supports nesting!)
                    else if (peek_next() == '*') {
                        advance(); // consume '/'
                        advance(); // consume '*'
                        int nesting_depth = 1;

                        while (nesting_depth > 0 && !is_at_end()) {
                            if (peek() == '/' && peek_next() == '*') {
                                advance();
                                advance();
                                ++nesting_depth;
                            } else if (peek() == '*' && peek_next() == '/') {
                                advance();
                                advance();
                                --nesting_depth;
                            } else {
                                if (peek() == '\n') {
                                    ++m_line;
                                    m_column = 0;
                                }
                                advance();
                            }
                        }

                        if (nesting_depth > 0) {
                            error_token("Unterminated block comment: missing closing '*/'.");
                            return;
                        }
                    } else {
                        return; // It is a division operator '/'
                    }
                    break;

                default:
                    return;
            }
        }
    }
};

} // namespace Faberge

#endif // LEXER_HPP