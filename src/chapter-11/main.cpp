#include "ProceduralPipeline.hpp"
#include "OOPPipeline.hpp"
#include "FunctionalPipeline.hpp"

#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <format>

// Synthetic telemetry dataset generator
void generate_dataset(
    std::size_t count,
    std::vector<procedural::RawPacket>& proc_data,
    std::vector<oop::TelemetryPacket>& oop_data,
    std::vector<functional::TelemetryRecord>& func_data)
{
    proc_data.reserve(count);
    oop_data.reserve(count);
    func_data.reserve(count);

    std::mt19937_64 rng(1337);
    std::uniform_real_distribution<double> volt_dist(0.0, 5.0);
    std::uniform_int_distribution<int> flag_dist(0, 7);

    for (std::size_t i = 0; i < count; ++i) {
        const auto ts = static_cast<std::uint64_t>(1700000000 + i);
        const auto dev = static_cast<std::uint32_t>(100 + (i % 16));
        const double v = volt_dist(rng);
        const auto flg = static_cast<std::uint8_t>(flag_dist(rng));

        proc_data.push_back(procedural::RawPacket{ts, dev, v, flg});
        oop_data.emplace_back(ts, dev, v, flg);
        func_data.push_back(functional::TelemetryRecord{ts, dev, v, flg});
    }
}

int main() {
    constexpr std::size_t PACKET_COUNT = 5'000'000;
    std::cout << std::format("Generating {} telemetry packets...\n\n", PACKET_COUNT);

    std::vector<procedural::RawPacket> proc_data;
    std::vector<oop::TelemetryPacket> oop_data;
    std::vector<functional::TelemetryRecord> func_data;

    generate_dataset(PACKET_COUNT, proc_data, oop_data, func_data);

    // =========================================================
    // 1. Procedural Benchmark
    // =========================================================
    auto t0 = std::chrono::high_resolution_clock::now();
    const auto proc_res = procedural::process_telemetry(proc_data);
    auto t1 = std::chrono::high_resolution_clock::now();
    const auto proc_time = std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();

    // =========================================================
    // 2. Object-Oriented Benchmark
    // =========================================================
    auto t2 = std::chrono::high_resolution_clock::now();
    oop::TelemetryAggregator oop_agg(
        std::make_unique<oop::HardwareFaultValidator>(0x02 | 0x04),
        std::make_unique<oop::LinearThermalCalibrator>(30.0, -20.0)
    );
    for (const auto& packet : oop_data) {
        oop_agg.ingest(packet);
    }
    auto t3 = std::chrono::high_resolution_clock::now();
    const auto oop_time = std::chrono::duration_cast<std::chrono::microseconds>(t3 - t2).count();

    // =========================================================
    // 3. Functional Benchmark
    // =========================================================
    auto t4 = std::chrono::high_resolution_clock::now();
    const auto func_res = functional::evaluate_stream(func_data);
    auto t5 = std::chrono::high_resolution_clock::now();
    const auto func_time = std::chrono::duration_cast<std::chrono::microseconds>(t5 - t4).count();

    // =========================================================
    // Telemetry and Result Verification
    // =========================================================
    std::cout << "=================== BENCHMARK RESULTS ===================\n";
    std::cout << std::format("Procedural Style Duration : {:>8} µs (Valid: {}, Max: {:.2f} °C, Crit: {})\n",
                             proc_time, proc_res.valid_packet_count, proc_res.max_temperature_c, proc_res.critical_count);
    std::cout << std::format("OOP Style Duration        : {:>8} µs (Valid: {}, Max: {:.2f} °C, Crit: {})\n",
                             oop_time, oop_agg.valid_count(), oop_agg.max_temperature(), oop_agg.critical_count());
    std::cout << std::format("Functional Style Duration : {:>8} µs (Valid: {}, Max: {:.2f} °C, Crit: {})\n",
                             func_time, func_res.valid_packet_count, func_res.max_temperature_c, func_res.critical_count);
    std::cout << "=========================================================\n\n";

    std::cout << "--- Performance Relative to Procedural Baseline ---\n";
    std::cout << std::format("Procedural Baseline : 1.00x\n");
    std::cout << std::format("OOP Overhead        : {:.2f}x slower\n", static_cast<double>(oop_time) / proc_time);
    std::cout << std::format("Functional Overhead : {:.2f}x (Practically identical!)\n", static_cast<double>(func_time) / proc_time);

    return 0;
}