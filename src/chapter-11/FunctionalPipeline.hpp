#ifndef FUNCTIONAL_PIPELINE_HPP
#define FUNCTIONAL_PIPELINE_HPP

#include <cstdint>
#include <vector>
#include <ranges>
#include <numeric>
#include <algorithm>
#include <span>

namespace functional {

// Immutable Data Record
struct TelemetryRecord {
    const std::uint64_t timestamp_ms;
    const std::uint32_t device_id;
    const double        raw_voltage;
    const std::uint8_t  flags;
};

enum class ThermalBand : std::uint8_t {
    Normal,
    Elevated,
    Critical
};

struct ProcessedReading {
    double      temperature_c;
    ThermalBand band;
};

struct PipelineSummary {
    std::size_t valid_packet_count{0};
    double      mean_temperature_c{0.0};
    double      max_temperature_c{0.0};
    std::size_t critical_count{0};
};

// Pure Predicates and Mapping Functions
constexpr auto is_valid_packet = [](const TelemetryRecord& r) noexcept -> bool {
    constexpr std::uint8_t ERROR_MASK = 0x02 | 0x04;
    return (r.flags & ERROR_MASK) == 0;
};

constexpr auto calibrate_and_classify = [](const TelemetryRecord& r) noexcept -> ProcessedReading {
    const double temp_c = (r.raw_voltage * 30.0) - 20.0;
    ThermalBand band = ThermalBand::Normal;
    if (temp_c >= 95.0) {
        band = ThermalBand::Critical;
    } else if (temp_c >= 75.0) {
        band = ThermalBand::Elevated;
    }
    return ProcessedReading{temp_c, band};
};

// Pure Functional Aggregator using std::accumulate (Fold Operation)
[[nodiscard]] inline PipelineSummary evaluate_stream(std::span<const TelemetryRecord> stream) {
    // 1. Construct a Lazy, Zero-Allocation Range Pipeline
    auto processed_view = stream 
                        | std::views::filter(is_valid_packet)
                        | std::views::transform(calibrate_and_classify);

    // 2. Pure Fold / Reduction
    struct AccumulatorState {
        std::size_t count{0};
        double      sum_temp{0.0};
        double      max_temp{-std::numeric_limits<double>::infinity()};
        std::size_t critical_count{0};
    };

    const auto [count, sum, max_val, crit_count] = std::accumulate(
        processed_view.begin(), processed_view.end(),
        AccumulatorState{},
        [](AccumulatorState acc, const ProcessedReading& reading) noexcept {
            return AccumulatorState{
                .count = acc.count + 1,
                .sum_temp = acc.sum_temp + reading.temperature_c,
                .max_temp = std::max(acc.max_temp, reading.temperature_c),
                .critical_count = acc.critical_count + (reading.band == ThermalBand::Critical ? 1 : 0)
            };
        }
    );

    return PipelineSummary{
        .valid_packet_count = count,
        .mean_temperature_c = (count > 0) ? (sum / static_cast<double>(count)) : 0.0,
        .max_temperature_c  = (count > 0) ? max_val : 0.0,
        .critical_count     = crit_count
    };
}

} // namespace functional

#endif // FUNCTIONAL_PIPELINE_HPP