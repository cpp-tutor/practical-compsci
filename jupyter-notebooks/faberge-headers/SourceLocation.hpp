#ifndef SOURCE_LOCATION_HPP
#define SOURCE_LOCATION_HPP

#include <string>
#include <string_view>
#include <cstdint>
#include <ostream>

namespace Faberge {

struct SourceLocation {
    std::string_view filename{"<anonymous>"};
    uint32_t line{1};
    uint32_t column{1};
    uint32_t offset{0};

    friend std::ostream& operator<<(std::ostream& os, const SourceLocation& loc) {
        os << loc.filename << ":" << loc.line << ":" << loc.column;
        return os;
    }
};

struct SourceSpan {
    SourceLocation start;
    SourceLocation end;
    std::string_view lexeme;
};

} // namespace Faberge

#endif // SOURCE_LOCATION_HPP