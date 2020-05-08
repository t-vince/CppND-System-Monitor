#include "processor.h"
#include "linux_parser.h"

// Return the aggregate CPU utilization
float Processor::Utilization() {
    // Get a more up-to-date value by removing the last known value (prev_)
    // from the all time value
    long alltime_total = LinuxParser::Jiffies();
    long alltime_idle = LinuxParser::IdleJiffies();
    long total = alltime_total - prev_total_;
    long idle = alltime_idle - prev_idle_;
    prev_total_ = alltime_total;
    prev_idle_ = alltime_idle;
    return static_cast<float>(total - idle) / total;
}