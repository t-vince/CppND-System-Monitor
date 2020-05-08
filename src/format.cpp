#include <string>
#include <iomanip>
#include <sstream>

#include "format.h"

using std::string;
using std::setw;
using std::setfill;

// TODO: Complete this helper function
// INPUT: Long int measuring seconds
// OUTPUT: HH:MM:SS
string Format::ElapsedTime(long seconds) {
    int hour_in_sec = 3600;
    int minute_in_sec = 60;

    int hour = seconds / hour_in_sec;
    long rest = seconds % hour_in_sec;
    int min = rest / minute_in_sec;
    int sec = rest % minute_in_sec;

    // Prepend with 0 if not 2 digit
    std::ostringstream ostream;
    ostream << setw(2) << setfill('0') << hour << ':'
            << setw(2) << setfill('0') << min << ':'
            << setw(2) << setfill('0') << sec;
    return ostream.str(); 
}