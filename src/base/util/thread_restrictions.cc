#include <glog/logging.h>
#include <gperftools/heap-checker.h>

#include "base/util/thread.h"
#include "base/util/threadlocal.h"
#include "base/util/thread_restrictions.h"

#ifdef ENABLE_THREAD_RESTRICTIONS

namespace base {

namespace {

struct LocalThreadRestrictions {
  LocalThreadRestrictions()
    : io_allowed(true),
      wait_allowed(true),
      singleton_allowed(true) {
  }

  bool io_allowed;
  bool wait_allowed;
  bool singleton_allowed;
};

LocalThreadRestrictions* LoadTLS() {
  BLOCK_STATIC_THREAD_LOCAL(LocalThreadRestrictions, local_thread_restrictions);
  return local_thread_restrictions;
}

} // anonymous namespace

bool ThreadRestrictions::SetIOAllowed(bool allowed) {
  bool previous_allowed = LoadTLS()->io_allowed;
  LoadTLS()->io_allowed = allowed;
  return previous_allowed;
}

void ThreadRestrictions::AssertIOAllowed() {
  CHECK(LoadTLS()->io_allowed)
    << "Function marked as IO-only was called from a thread that "
    << "disallows IO!  If this thread really should be allowed to "
    << "make IO calls, adjust the call to "
    << "base::ThreadRestrictions::SetIOAllowed() in this thread's "
    << "startup. "
    << (Thread::current_thread() ? Thread::current_thread()->ToString() : "(not a base::Thread)");
}

bool ThreadRestrictions::SetWaitAllowed(bool allowed) {
  bool previous_allowed = LoadTLS()->wait_allowed;
  LoadTLS()->wait_allowed = allowed;
  return previous_allowed;
}

void ThreadRestrictions::AssertWaitAllowed() {
  CHECK(LoadTLS()->wait_allowed)
    << "Waiting is not allowed to be used on this thread to prevent "
    << "server-wide latency aberrations and deadlocks. "
    << (Thread::current_thread() ? Thread::current_thread()->ToString() : "(not a base::Thread)");
}

} // namespace base
#endif
