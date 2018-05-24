#pragma once
#include <cstdint>

constexpr int32_t V = 4;
constexpr int32_t H = 4;
constexpr int32_t W = 6;
constexpr int32_t P = 24;

constexpr int32_t sizeof_flit = 32;
constexpr int32_t packet_length = 5;
constexpr int32_t sizeof_packet = sizeof_flit * packet_length;

constexpr int32_t inbuf_capacity = 12;
constexpr int32_t swtbuf_capacity = 5;
constexpr int32_t colbuf_capacity = 20;

constexpr int32_t pipeline_stages = 2;

constexpr int32_t nodebuf_capacity = 256;

constexpr double clk_freq = 437500000.0;
constexpr double clk_cycle = 1 / clk_freq;

constexpr double sim_start_time = 1;

enum class log_levels : int32_t { critical = 0, warn, info, debug };
enum class traffic_patterns : int32_t { uniform = 0 };

constexpr double period = (packet_length + 1) * clk_cycle;
