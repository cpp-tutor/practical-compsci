#ifndef CONTRACTS_HPP
#define CONTRACTS_HPP

#include <iostream>
#include <string_view>
#include <source_location>
#include <stdexcept>
#include <format>

namespace proof {

class ContractViolationException : public std::logic_error {
public:
    ContractViolationException(
        std::string_view contract_type,
        std::string_view condition_str,
        const std::source_location& loc)
        : std::logic_error(std::format(
            "[{}] Violation: '{}' at {}:{} (Function: '{}')",
            contract_type,
            condition_str,
            loc.file_name(),
            loc.line(),
            loc.function_name())) {}
};

inline void contract_assert(
    bool condition,
    std::string_view contract_type,
    std::string_view condition_str,
    const std::source_location loc = std::source_location::current())
{
    if (!condition) {
        throw ContractViolationException(contract_type, condition_str, loc);
    }
}

// Contract Macros
#define CONTRACT_EXPECTS(cond) \
    proof::contract_assert((cond), "Precondition (Requires)", #cond)

#define CONTRACT_ENSURES(cond) \
    proof::contract_assert((cond), "Postcondition (Ensures)", #cond)

#define CONTRACT_INVARIANT(cond) \
    proof::contract_assert((cond), "Class Invariant", #cond)

} // namespace proof

#endif // CONTRACTS_HPP