#ifndef PROCEDURAL_PIPELINE_HPP
#define PROCEDURAL_PIPELINE_HPP

#include <cstdint>
#include <vector>
#include <limits>
#include <span>
#include <algorithm>

namespace procedural {

// Plain Old Data (POD) Packet
struct RawPacket {
    std::uint64_t timestamp_ms;
    std::uint32_t device_id;
    double        raw_voltage;
    std::uint8_t  flags;
};

enum class ThermalBand : std::uint8_t {
    Normal,
    Elevated,
    Critical
};

struct IngestionSummary {
    std::size_t valid_packet_count{0};
    double      mean_temperature_c{0.0};
    double      max_temperature_c{-std::numeric_limits<double>::infinity()};
    std::size_t critical_count{0};
};

// Procedural verification
[[nodiscard]] constexpr bool is_packet_valid(const RawPacket& p) noexcept {
    constexpr std::uint8_t ERROR_MASK = 0x02 | 0x04; // Parity Error | Signal Clip
    return (p.flags & ERROR_MASK) == 0;
}

// Procedural calibration
[[nodiscard]] constexpr double calibrate_voltage_to_celsius(double voltage) noexcept {
    return (voltage * 30.0) - 20.0;
}

// Procedural classification
[[nodiscard]] constexpr ThermalBand classify_temperature(double temp_c) noexcept {
    if (temp_c >= 95.0) return ThermalBand::Critical;
    if (temp_c >= 75.0) return ThermalBand::Elevated;
    return ThermalBand::Normal;
}

// Monolithic, high-speed procedural aggregation loop
[[nodiscard]] inline IngestionSummary process_telemetry(std::span<const RawPacket> packets) noexcept {
    IngestionSummary summary{};
    double total_temp = 0.0;

    for (std::size_t i = 0; i < packets.size(); ++i) {
        const auto& packet = packets[i];
        
        // 1. Validate
        if (!is_packet_valid(packet)) {
            continue;
        }

        // 2. Calibrate
        const double temp_c = calibrate_voltage_to_celsius(packet.raw_voltage);

        // 3. Classify
        const ThermalBand band = classify_temperature(temp_c);

        // 4. Mutate Accumulators
        ++summary.valid_packet_count;
        total_temp += temp_c;
        if (temp_c > summary.max_temperature_c) {
            summary.max_temperature_c = temp_c;
        }
        if (band == ThermalBand::Critical) {
            ++summary.critical_count;
        }
    }

    if (summary.valid_packet_count > 0) {
        summary.mean_temperature_c = total_temp / static_cast<double>(summary.valid_packet_count);
    } else {
        summary.max_temperature_c = 0.0;
    }

    return summary;
}

} // namespace procedural

#endif // PROCEDURAL_PIPELINE_HPP