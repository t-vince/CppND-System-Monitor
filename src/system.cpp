#include <unistd.h>
#include <cstddef>
#include <set>
#include <string>
#include <vector>

#include "linux_parser.h"
#include "process.h"
#include "processor.h"
#include "system.h"

using std::set;
using std::size_t;
using std::string;
using std::vector;

Processor& System::Cpu() { return cpu_; }

// Return a container composed of the system's processes
vector<Process>& System::Processes() {
    // Clear to ensure no killed processes remain
    processes_.clear();
    processes_.shrink_to_fit();

    // For each process that is not in the processes_ vector, add it
    for (int pid : LinuxParser::Pids()) {
        Process process = Process(pid);
        processes_.emplace_back(process);
    }

    // Sort with highest (avg) cpu first and return
    std::sort(processes_.begin(), processes_.end(), [](const Process a, const Process b) {
            return b < a;
    });
    return processes_; 
}

// Return the system's kernel identifier (string)
std::string System::Kernel() const { return kernel_; }

// Return the system's memory utilization
float System::MemoryUtilization() const { return LinuxParser::MemoryUtilization(); }

// Return the operating system name
std::string System::OperatingSystem() const { return os_; }

// Return the number of processes actively running on the system
int System::RunningProcesses() const { return LinuxParser::RunningProcesses(); }

// Return the total number of processes on the system
int System::TotalProcesses() const { return LinuxParser::TotalProcesses(); }

// Return the number of seconds since the system started running
long int System::UpTime() const { return LinuxParser::UpTime(); }