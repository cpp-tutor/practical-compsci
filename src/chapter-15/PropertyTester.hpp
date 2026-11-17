#ifndef PROPERTY_TESTER_HPP
#define PROPERTY_TESTER_HPP

#include <random>
#include <vector>
#include <string>
#include <functional>
#include <iostream>
#include <format>
#include <concepts>

namespace proof {

// Type Generator Concept
template <typename T>
struct Generator;

// Specialisation for int32_t
template <>
struct Generator<int> {
    static int generate(std::mt19937_64& rng) {
        std::uniform_int_distribution<int> dist(-10'000, 10'000);
        return dist(rng);
    }
};

// Specialisation for std::vector<int>
template <>
struct Generator<std::vector<int>> {
    static std::vector<int> generate(std::mt19937_64& rng) {
        std::uniform_int_distribution<std::size_t> len_dist(0, 50);
        std::uniform_int_distribution<int> val_dist(-500, 500);

        const std::size_t len = len_dist(rng);
        std::vector<int> result(len);
        for (std::size_t i = 0; i < len; ++i) {
            result[i] = val_dist(rng);
        }
        return result;
    }
};

class PropertyTester {
public:
    explicit PropertyTester(std::size_t iterations = 500, std::uint64_t seed = 1337)
        : m_iterations(iterations), m_rng(seed) {}

    // Verify a unary property: ∀ x ∈ T, Predicate(x) == true
    template <typename T, typename PropertyFunc>
    bool check(std::string_view property_name, PropertyFunc prop) {
        std::cout << std::format("Verifying Property: \"{}\" across {} cases... ", property_name, m_iterations);

        for (std::size_t i = 0; i < m_iterations; ++i) {
            T sample = Generator<T>::generate(m_rng);
            try {
                if (!prop(sample)) {
                    std::cout << "\n>> FAILED (Counterexample Discovered)!\n";
                    print_counterexample(sample);
                    return false;
                }
            } catch (const std::exception& e) {
                std::cout << "\n>> FAILED WITH EXCEPTION!\n";
                std::cout << std::format("Exception: {}\n", e.what());
                print_counterexample(sample);
                return false;
            }
        }

        std::cout << "[PASSED]\n";
        return true;
    }

    // Verify a binary property: ∀ x, y ∈ T, Predicate(x, y) == true
    template <typename T, typename PropertyFunc>
    bool check_binary(std::string_view property_name, PropertyFunc prop) {
        std::cout << std::format("Verifying Binary Property: \"{}\" across {} cases... ", property_name, m_iterations);

        for (std::size_t i = 0; i < m_iterations; ++i) {
            T sample_a = Generator<T>::generate(m_rng);
            T sample_b = Generator<T>::generate(m_rng);
            try {
                if (!prop(sample_a, sample_b)) {
                    std::cout << "\n>> FAILED (Counterexample Discovered)!\n";
                    return false;
                }
            } catch (const std::exception& e) {
                std::cout << "\n>> FAILED WITH EXCEPTION: " << e.what() << "\n";
                return false;
            }
        }

        std::cout << "[PASSED]\n";
        return true;
    }

private:
    template <typename T>
    void print_counterexample(const T& val) {
        std::cout << "Counterexample state: " << val << "\n";
    }

    void print_counterexample(const std::vector<int>& vec) {
        std::cout << "Counterexample vector size: " << vec.size() << " elements: [";
        for (std::size_t i = 0; i < std::min(vec.size(), std::size_t(10)); ++i) {
            std::cout << vec[i] << (i + 1 < vec.size() ? ", " : "");
        }
        if (vec.size() > 10) std::cout << "...";
        std::cout << "]\n";
    }

    std::size_t    m_iterations;
    std::mt19937_64 m_rng;
};

} // namespace proof

#endif // PROPERTY_TESTER_HPP