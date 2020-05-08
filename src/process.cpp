#include <unistd.h>
#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"
#include "process.h"

using std::string;
using std::to_string;
using std::vector;

Process::Process(int pid) : pid_(pid) {
    if (pid_ < 0)
        throw std::invalid_argument("A process ID can not be negative");
    user_ = LinuxParser::User(pid_);
    command_ = LinuxParser::Command(pid_);
    Update();
}

int Process::Pid() const { return pid_; }

// Return this process's CPU utilization
float Process::CpuUtilization() const { return cpu_utilization_; }

void Process::Update() {
    uptime_ = LinuxParser::UpTime(pid_);
    float total_cpu_time = LinuxParser::ActiveJiffies(pid_) / sysconf(_SC_CLK_TCK);
    // Note: ncurses_display already multiplies by 100
    cpu_utilization_ = total_cpu_time / uptime_;
}

string Process::Command() const { return command_; }

// Return this process's memory utilization
string Process::Ram() const { return LinuxParser::Ram(pid_); }

string Process::User() const { return user_; }

// Return the age of this process (in seconds)
long int Process::UpTime() const { return uptime_; }

// Overload the "less than" comparison operator for Process objects
// Order by cpu utilization
bool Process::operator<(Process const& a) const { 
    return CpuUtilization() < a.CpuUtilization();
}