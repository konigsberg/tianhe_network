#include "macros.h"

constexpr double warm_up_time = 0.000000150;
constexpr double record_start_time = sim_start_time + warm_up_time;
constexpr double injection_rate = 0.2;
constexpr bool debug_mode = false;
constexpr traffic_patterns traffic_pattern = traffic_patterns::uniform;
constexpr log_levels log_lvl = log_levels::critical;
