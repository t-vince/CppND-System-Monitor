#include <dirent.h>
#include <unistd.h>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::stof;
using std::stol;
using std::stoi;
using std::string;
using std::to_string;
using std::vector;

// DONE: An example of how to read data from the filesystem
string LinuxParser::OperatingSystem() {
  string line;
  string key;
  string value;
  std::ifstream filestream(kOSPath);
  if (filestream.is_open()) {
    while (std::getline(filestream, line)) {
      std::replace(line.begin(), line.end(), ' ', '_');
      std::replace(line.begin(), line.end(), '=', ' ');
      std::replace(line.begin(), line.end(), '"', ' ');
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "PRETTY_NAME") {
          std::replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
  }
  return value;
}

// DONE: An example of how to read data from the filesystem
string LinuxParser::Kernel() {
  string os, version, kernel;
  string line;
  std::ifstream stream(kProcDirectory + kVersionFilename);
  if (stream.is_open()) {
    std::getline(stream, line);
    std::istringstream linestream(line);
    linestream >> os >> version >> kernel;
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;
  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.push_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

// Read and return the system memory utilization
float LinuxParser::MemoryUtilization() {
  float mem_total{0.0};
  float mem_free{0.0};
  string line;
  string key;
  string value;
  string unit;
  std::ifstream stream(kProcDirectory + kMeminfoFilename);
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value >> unit) {
        if (key == "MemTotal:") {
          mem_total = stof(value);
        } else if (key == "MemFree:") {
          mem_free = stof(value);
        }
      }
    }
    if (mem_total > 0)
      return (mem_total - mem_free) / mem_total;
  }
  return 0.0; 
}

// Read and return the system uptime
long LinuxParser::UpTime() {
  string uptime;
  string idle_time;
  std::ifstream stream(kProcDirectory + kUptimeFilename);
  if (stream.is_open()) {
    stream >> uptime >> idle_time;
    return stol(uptime);
  }
  return 0; 
}

// Read and return the number of jiffies for the system
long LinuxParser::Jiffies() { return ActiveJiffies() + IdleJiffies(); }

// Read and return the number of active jiffies for a PID
long LinuxParser::ActiveJiffies(int pid) { 
  std::ifstream stream(kProcDirectory + "/" + to_string(pid) + kStatFilename);
  string line;
  string value;
  long jiffies{0};
  // Including children processes; Without Hz division
  if (stream.is_open()) {
      std::getline(stream, line);
      std::istringstream linestream(line);
      int i{0};
      while (linestream >> value) {
        i++;
        // utime, stime,cutime and cstime
        if (i >= 13 && i <= 16)
          jiffies += stol(value);
      }
  }
  return jiffies; 
}

// Read and return the number of active jiffies for the system
long LinuxParser::ActiveJiffies() {
  vector<string> cpu_stats = CpuUtilization();
  long total{0};
  for (const auto state : {CPUStates::kUser_, CPUStates::kNice_, 
                          CPUStates::kSystem_, CPUStates::kIRQ_, 
                          CPUStates::kSoftIRQ_, CPUStates::kSteal_}) {
    total += stol(cpu_stats.at(state));
  }
  return total;
}

// Read and return the number of idle jiffies for the system
long LinuxParser::IdleJiffies() {
  vector<string> cpu_stats = CpuUtilization();
  long total{0};
  for (const auto state : {CPUStates::kIdle_, CPUStates::kIOwait_})
    total += stol(cpu_stats.at(state));
  return total;
}

// Read and return CPU utilization
vector<string> LinuxParser::CpuUtilization() { 
  std::ifstream stream(kProcDirectory + kStatFilename);
  vector<string> cpu_stat;
  string value;
  string line;
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      while (linestream >> value) {
        if (value == "cpu")
          continue;
        cpu_stat.push_back(value);
      }
    }
  }  
  return cpu_stat; 
}

// Read and return the total number of processes
int LinuxParser::TotalProcesses() { 
  std::ifstream stream(kProcDirectory + kStatFilename);
  string key;
  string value;
  string line;
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "processes") 
          return stoi(value);
      }
    }
  }  
  return 0;
}

// Read and return the number of running processes
int LinuxParser::RunningProcesses() {
  std::ifstream stream(kProcDirectory + kStatFilename);
  string key;
  string value;
  string line;
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "procs_running") 
          return stoi(value);
      }
    }
  }  
  return 0;
}

// Read and return the command associated with a process
string LinuxParser::Command(int pid) {
  std::ifstream stream(kProcDirectory + "/" + to_string(pid) + kCmdlineFilename);
  string line;
  if (stream.is_open()) {
    if (std::getline(stream, line))
      return line;
  } 
  return string(); 
}

// Read and return the memory used by a process
string LinuxParser::Ram(int pid) { 
  std::ifstream stream(kProcDirectory + "/" + to_string(pid) + kStatusFilename);
  string line;
  string key;
  string value;
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "VmSize:") {
          return to_string(stol(value)/1000);
        }
      }
    }
  }
  return string();
}

// Read and return the user ID associated with a process
string LinuxParser::Uid(int pid) { 
  std::ifstream stream(kProcDirectory + "/" + to_string(pid) + kStatusFilename);
  string line;
  string key;
  string value;
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      std::istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == "Uid:")
          return value;
      }
    }
  }
  // User ID not found, return uid of nobody
  return "65534";
}

// Read and return the user associated with a process from the password file
string LinuxParser::User(int pid) { 
  std::ifstream stream(kPasswordPath);
  string uid = Uid(pid);
  string line;
  string user;
  string tmpuid;
  string x;
  if (stream.is_open()) {
    while (std::getline(stream, line)) {
      // Remove unwanted characters
      std::replace(line.begin(), line.end(), ':', ' ');
      std::istringstream linestream(line);
      while (linestream >> user >> x >> tmpuid) {
        if (tmpuid == uid)
          return user;
      }
    }
  }
  // If no user found
  return "Undefined"; 
}

// Return the uptime in no of clock ticks per sec
long LinuxParser::UpTime(int pid) { 
  std::ifstream stream(kProcDirectory + "/" + to_string(pid) + kStatFilename);
  if (stream.is_open()) {
    string line;
    string value;
    std::getline(stream, line);
    std::istringstream linestream(line);
    // It is defined as "The time the process started after boot"
    // To get the uptime, we should subtract it from total system uptime
    // and divide by _SC_CLK_TCK to get seconds
    int i{0};
    while (linestream >> value) {
      i++;
      if (i == 22) {
        return UpTime() - (std::stol(value) / sysconf(_SC_CLK_TCK));
        break;
      }
     }
  }
  return 0;
}