#include "Contracts.hpp"
#include "PropertyTester.hpp"
#include "ContractedStack.hpp"

#include <algorithm>
#include <numeric>
#include <iostream>

// Sample Pure Function to Formally Test: Vector Reversal
std::vector<int> reverse_list(std::vector<int> xs) {
    std::reverse(xs.begin(), xs.end());
    return xs;
}

// Sample Pure Function to Formally Test: List Concatenation
std::vector<int> append_lists(std::vector<int> xs, const std::vector<int>& ys) {
    xs.insert(xs.end(), ys.begin(), ys.end());
    return xs;
}

int main() {
    std::cout << "========================================================\n";
    std::cout << "1. Design-by-Contract (DbC) Runtime Verification\n";
    std::cout << "========================================================\n";

    proof::ContractedStack<int> stack(3); // Stack with capacity 3
    stack.push(10);
    stack.push(20);
    stack.push(30);

    std::cout << std::format("Stack size: {} / {}\n", stack.size(), stack.capacity());

    try {
        std::cout << "Attempting to push to a full stack (deliberate Precondition violation)...\n";
        stack.push(40); // Violates Precondition: !is_full()
    } catch (const proof::ContractViolationException& e) {
        std::cout << ">> Caught Contract Violation as expected!\n";
        std::cout << ">> " << e.what() << "\n";
    }

    std::cout << "\n========================================================\n";
    std::cout << "2. Property-Based Mathematical Proofs (PBT)\n";
    std::cout << "========================================================\n";

    proof::PropertyTester tester(1000); // Test 1000 generated cases per property

    // Property 1: Involution of Reverse -> reverse(reverse(xs)) == xs
    tester.check<std::vector<int>>(
        "Involution of Reverse: reverse(reverse(xs)) ≡ xs",
        [](const std::vector<int>& xs) {
            return reverse_list(reverse_list(xs)) == xs;
        }
    );

    // Property 2: Length Preservation of Append -> |append(xs, ys)| == |xs| + |ys|
    tester.check_binary<std::vector<int>>(
        "Length Preservation: |append(xs, ys)| ≡ |xs| + |ys|",
        [](const std::vector<int>& xs, const std::vector<int>& ys) {
            auto combined = append_lists(xs, ys);
            return combined.size() == (xs.size() + ys.size());
        }
    );

    // Property 3: Identity Element of Append -> append(xs, []) == xs
    tester.check<std::vector<int>>(
        "Right Identity of Append: append(xs, []) ≡ xs",
        [](const std::vector<int>& xs) {
            return append_lists(xs, {}) == xs;
        }
    );

    // Property 4: Sorting Idempotence -> sort(sort(xs)) == sort(xs)
    tester.check<std::vector<int>>(
        "Idempotence of Sorting: sort(sort(xs)) ≡ sort(xs)",
        [](std::vector<int> xs) {
            std::vector<int> once = xs;
            std::sort(once.begin(), once.end());
            std::vector<int> twice = once;
            std::sort(twice.begin(), twice.end());
            return once == twice;
        }
    );

    return 0;
}