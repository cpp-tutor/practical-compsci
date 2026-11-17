#ifndef DIAGNOSTIC_HPP
#define DIAGNOSTIC_HPP

#include "SourceLocation.hpp"
#include <iostream>
#include <string_view>
#include <sstream>
#include <vector>

namespace Faberge {

class DiagnosticEngine {
public:
    static void report_error(std::string_view source, 
                             SourceLocation loc, 
                             std::string_view message) {
        // ANSI escape codes for modern terminal formatting
        constexpr std::string_view ANSI_RED_BOLD = "\033[1;31m";
        constexpr std::string_view ANSI_BOLD     = "\033[1m";
        constexpr std::string_view ANSI_BLUE     = "\033[1;34m";
        constexpr std::string_view ANSI_RESET    = "\033[0m";

        std::cerr << ANSI_RED_BOLD << "error: " << ANSI_RESET 
                  << ANSI_BOLD << message << ANSI_RESET << "\n"
                  << "  --> " << loc << "\n";

        // Extract the entire offending source line
        std::string_view line_str = extract_line(source, loc.offset);
        
        std::string line_num_str = std::to_string(loc.line);
        std::string padding(line_num_str.length(), ' ');

        std::cerr << ANSI_BLUE << padding << " |\n"
                  << line_num_str << " | " << ANSI_RESET << line_str << "\n"
                  << ANSI_BLUE << padding << " | " << ANSI_RED_BOLD;

        // Render the visual caret underline (^~~~~)
        for (uint32_t i = 1; i < loc.column; ++i) {
            std::cerr << ' ';
        }
        std::cerr << "^" << ANSI_RESET << "\n\n";
    }

private:
    static std::string_view extract_line(std::string_view source, uint32_t offset) {
        if (offset >= source.size()) offset = source.empty() ? 0 : source.size() - 1;

        // Trace backward to find line start
        size_t start = offset;
        while (start > 0 && source[start - 1] != '\n') {
            --start;
        }

        // Trace forward to find line end
        size_t end = offset;
        while (end < source.size() && source[end] != '\n' && source[end] != '\r') {
            ++end;
        }

        return source.substr(start, end - start);
    }
};

} // namespace Faberge

#endif // DIAGNOSTIC_HPP