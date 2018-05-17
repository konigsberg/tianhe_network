#pragma once
#include <cstdint>

const int32_t V = 6;
const int32_t H = 4;
const int32_t W = 6;
const int32_t P = 24;

const int32_t FlitSize = 32;
const int32_t PacketLength = 5;
const int32_t PacketSize = 160;

const int32_t InputBufferDepth = 12;
const int32_t SwitchBufferDepth = 5;
const int32_t ColumnBufferDepth = 20;
const int32_t PipelineDepth = 2;
const int32_t NodeBufferDepth = 256;

const int32_t TrafficPattern = 1;
const int32_t UniformTraffic = 1;

const double Frequency = 437500000.0;
const double ClockCycle = 2.28571428571e-09;

const double SimStartTime = 1;
const double RecordStartTime = 1;

const double InjectionRate = 1.0;
enum log_levels { error = 0, warning, info, debug };
