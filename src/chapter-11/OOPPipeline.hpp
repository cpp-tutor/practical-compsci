#ifndef OOP_PIPELINE_HPP
#define OOP_PIPELINE_HPP

#include <cstdint>
#include <memory>
#include <vector>
#include <limits>
#include <stdexcept>

namespace oop {

enum class ThermalBand : std::uint8_t {
    Normal,
    Elevated,
    Critical
};

// Domain Object: Encapsulated Telemetry Packet
class TelemetryPacket {
public:
    TelemetryPacket(std::uint64_t ts, std::uint32_t dev_id, double voltage, std::uint8_t flags)
        : m_timestamp_ms(ts), m_device_id(dev_id), m_raw_voltage(voltage), m_flags(flags) {}

    [[nodiscard]] std::uint64_t timestamp() const noexcept { return m_timestamp_ms; }
    [[nodiscard]] std::uint32_t device_id() const noexcept { return m_device_id; }
    [[nodiscard]] double raw_voltage() const noexcept { return m_raw_voltage; }
    [[nodiscard]] std::uint8_t flags() const noexcept { return m_flags; }

private:
    std::uint64_t m_timestamp_ms;
    std::uint32_t m_device_id;
    double        m_raw_voltage;
    std::uint8_t  m_flags;
};

// Abstract Polymorphic Validator Interface
class IPacketValidator {
public:
    virtual ~IPacketValidator() = default;
    [[nodiscard]] virtual bool validate(const TelemetryPacket& packet) const = 0;
};

class HardwareFaultValidator : public IPacketValidator {
public:
    explicit HardwareFaultValidator(std::uint8_t rejection_mask) : m_rejection_mask(rejection_mask) {}

    [[nodiscard]] bool validate(const TelemetryPacket& packet) const override {
        return (packet.flags() & m_rejection_mask) == 0;
    }

private:
    std::uint8_t m_rejection_mask;
};

// Abstract Polymorphic Calibrator Interface
class ICalibrator {
public:
    virtual ~ICalibrator() = default;
    [[nodiscard]] virtual double calibrate(double raw_input) const = 0;
};

class LinearThermalCalibrator : public ICalibrator {
public:
    LinearThermalCalibrator(double scale, double offset) : m_scale(scale), m_offset(offset) {}

    [[nodiscard]] double calibrate(double raw_input) const override {
        return (raw_input * m_scale) + m_offset;
    }

private:
    double m_scale;
    double m_offset;
};

// Stateful Aggregator Class
class TelemetryAggregator {
public:
    TelemetryAggregator(std::unique_ptr<IPacketValidator> validator,
                        std::unique_ptr<ICalibrator> calibrator)
        : m_validator(std::move(validator)),
          m_calibrator(std::move(calibrator)) 
    {
        if (!m_validator || !m_calibrator) {
            throw std::invalid_argument("Validator and Calibrator cannot be null.");
        }
    }

    void ingest(const TelemetryPacket& packet) {
        if (!m_validator->validate(packet)) {
            return;
        }

        const double temp_c = m_calibrator->calibrate(packet.raw_voltage());
        const ThermalBand band = classify(temp_c);

        ++m_valid_count;
        m_total_temp += temp_c;
        if (temp_c > m_max_temp) {
            m_max_temp = temp_c;
        }
        if (band == ThermalBand::Critical) {
            ++m_critical_count;
        }
    }

    [[nodiscard]] std::size_t valid_count() const noexcept { return m_valid_count; }
    [[nodiscard]] std::size_t critical_count() const noexcept { return m_critical_count; }
    [[nodiscard]] double max_temperature() const noexcept { 
        return m_valid_count > 0 ? m_max_temp : 0.0; 
    }
    [[nodiscard]] double mean_temperature() const noexcept {
        return m_valid_count > 0 ? (m_total_temp / static_cast<double>(m_valid_count)) : 0.0;
    }

private:
    [[nodiscard]] ThermalBand classify(double temp_c) const noexcept {
        if (temp_c >= 95.0) return ThermalBand::Critical;
        if (temp_c >= 75.0) return ThermalBand::Elevated;
        return ThermalBand::Normal;
    }

    std::unique_ptr<IPacketValidator> m_validator;
    std::unique_ptr<ICalibrator>      m_calibrator;

    std::size_t m_valid_count{0};
    double      m_total_temp{0.0};
    double      m_max_temp{-std::numeric_limits<double>::infinity()};
    std::size_t m_critical_count{0};
};

} // namespace oop

#endif // OOP_PIPELINE_HPP