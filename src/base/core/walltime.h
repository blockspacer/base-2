#ifndef BASE_CORE_WALLTIME_H_
#define BASE_CORE_WALLTIME_H_

#include <sys/time.h>

#include <glog/logging.h>
#include <string>
using std::string;

#include "base/core/integral_types.h"

typedef double WallTime;

// Append result to a supplied string.
// If an error occurs during conversion 'dst' is not modified.
void StringAppendStrftime(std::string* dst,
                                 const char* format,
                                 time_t when,
                                 bool local);

// Return the local time as a string suitable for user display.
std::string LocalTimeAsString();

// Similar to the WallTime_Parse, but it takes a boolean flag local as
// argument specifying if the time_spec is in local time or UTC
// time. If local is set to true, the same exact result as
// WallTime_Parse is returned.
bool WallTime_Parse_Timezone(const char* time_spec,
                                    const char* format,
                                    const struct tm* default_time,
                                    bool local,
                                    WallTime* result);

// Return current time in seconds as a WallTime.
WallTime WallTime_Now();

typedef int64 MicrosecondsInt64;

namespace walltime_internal {


inline MicrosecondsInt64 GetClockTimeMicros(clockid_t clock) {
  timespec ts;
  clock_gettime(clock, &ts);
  return ts.tv_sec * 1e6 + ts.tv_nsec / 1e3;
}

} // namespace walltime_internal

// Returns the time since the Epoch measured in microseconds.
inline MicrosecondsInt64 GetCurrentTimeMicros() {
  return walltime_internal::GetClockTimeMicros(CLOCK_REALTIME);
}

// Returns the time since some arbitrary reference point, measured in microseconds.
// Guaranteed to be monotonic (and therefore useful for measuring intervals)
inline MicrosecondsInt64 GetMonoTimeMicros() {
  return walltime_internal::GetClockTimeMicros(CLOCK_MONOTONIC);
}

// Returns the time spent in user CPU on the current thread, measured in microseconds.
inline MicrosecondsInt64 GetThreadCpuTimeMicros() {
  return walltime_internal::GetClockTimeMicros(CLOCK_THREAD_CPUTIME_ID);
}

// A CycleClock yields the value of a cycle counter that increments at a rate
// that is approximately constant.
class CycleClock {
 public:
  // Return the value of the counter.
  static inline int64 Now();

 private:
  CycleClock();
};

#include "base/core/cycleclock-inl.h"  // inline method bodies
#endif  // GUTIL_WALLTIME_H_
