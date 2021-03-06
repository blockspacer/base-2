#ifndef BASE_UTIL_LOCKS_H
#define BASE_UTIL_LOCKS_H

#include <algorithm>
#include <glog/logging.h>
#include <mutex>

#include "base/core/atomicops.h"
#include "base/core/dynamic_annotations.h"
#include "base/core/macros.h"
#include "base/core/port.h"
#include "base/core/spinlock.h"
#include "base/core/sysinfo.h"

#include "base/util/rw_semaphore.h"

namespace base {

using core::subtle::Acquire_CompareAndSwap;
using core::subtle::NoBarrier_Load;
using core::subtle::Release_Store;

// Wrapper around the Google SpinLock class to adapt it to the method names
// expected by Boost.
class simple_spinlock {
 public:
  simple_spinlock() {}

  void lock() {
    l_.Lock();
  }

  void unlock() {
    l_.Unlock();
  }

  bool try_lock() {
    return l_.TryLock();
  }

  // Return whether the lock is currently held.
  //
  // This state can change at any instant, so this is only really useful
  // for assertions where you expect to hold the lock. The success of
  // such an assertion isn't a guarantee that the current thread is the
  // holder, but the failure of such an assertion _is_ a guarantee that
  // the current thread is _not_ holding the lock!
  bool is_locked() {
    return l_.IsHeld();
  }

 private:
  core::SpinLock l_;

  DISALLOW_COPY_AND_ASSIGN(simple_spinlock);
};

struct padded_spinlock : public simple_spinlock {
  char padding[CACHELINE_SIZE - (sizeof(simple_spinlock) % CACHELINE_SIZE)];
};

// Reader-writer lock.
// This is functionally equivalent to rw_semaphore in rw_semaphore.h, but should be
// used whenever the lock is expected to only be acquired on a single thread.
// It adds TSAN annotations which will detect misuse of the lock, but those
// annotations also assume that the same thread the takes the lock will unlock it.
//
// See rw_semaphore.h for documentation on the individual methods where unclear.
class rw_spinlock {
 public:
  rw_spinlock() {
    ANNOTATE_RWLOCK_CREATE(this);
  }
  ~rw_spinlock() {
    ANNOTATE_RWLOCK_DESTROY(this);
  }

  void lock_shared() {
    sem_.lock_shared();
    ANNOTATE_RWLOCK_ACQUIRED(this, 0);
  }

  void unlock_shared() {
    ANNOTATE_RWLOCK_RELEASED(this, 0);
    sem_.unlock_shared();
  }

  bool try_lock() {
    bool ret = sem_.try_lock();
    if (ret) {
      ANNOTATE_RWLOCK_ACQUIRED(this, 1);
    }
    return ret;
  }

  void lock() {
    sem_.lock();
    ANNOTATE_RWLOCK_ACQUIRED(this, 1);
  }

  void unlock() {
    ANNOTATE_RWLOCK_RELEASED(this, 1);
    sem_.unlock();
  }

  bool is_write_locked() const {
    return sem_.is_write_locked();
  }

  bool is_locked() const {
    return sem_.is_locked();
  }

 private:
  rw_semaphore sem_;
};

// A reader-writer lock implementation which is biased for use cases where
// the write lock is taken infrequently, but the read lock is used often.
//
// Internally, this creates N underlying mutexes, one per CPU. When a thread
// wants to lock in read (shared) mode, it locks only its own CPU's mutex. When it
// wants to lock in write (exclusive) mode, it locks all CPU's mutexes.
//
// This means that in the read-mostly case, different readers will not cause any
// cacheline contention.
//
// Usage:
//   percpu_rwlock mylock;
//
//   // Lock shared:
//   {
//     boost::shared_lock<rw_spinlock> lock(mylock.get_lock());
//     ...
//   }
//
//   // Lock exclusive:
//
//   {
//     boost::lock_guard<percpu_rwlock> lock(mylock);
//     ...
//   }
class percpu_rwlock {
 public:
  percpu_rwlock() {
    n_cpus_ = core::MaxCPUIndex() + 1;
    CHECK_GT(n_cpus_, 0);
    locks_ = new padded_lock[n_cpus_];
  }

  ~percpu_rwlock() {
    delete [] locks_;
  }

  rw_spinlock &get_lock() {
    int cpu = sched_getcpu();
    CHECK_LT(cpu, n_cpus_);
    return locks_[cpu].lock;
  }

  bool try_lock() {
    for (int i = 0; i < n_cpus_; i++) {
      if (!locks_[i].lock.try_lock()) {
        while (i--) {
          locks_[i].lock.unlock();
        }
        return false;
      }
    }
    return true;
  }

  // Return true if this lock is held on any CPU.
  // See simple_spinlock::is_locked() for details about where this is useful.
  bool is_locked() const {
    for (int i = 0; i < n_cpus_; i++) {
      if (locks_[i].lock.is_locked()) return true;
    }
    return false;
  }

  void lock() {
    for (int i = 0; i < n_cpus_; i++) {
      locks_[i].lock.lock();
    }
  }

  void unlock() {
    for (int i = 0; i < n_cpus_; i++) {
      locks_[i].lock.unlock();
    }
  }

  // Returns the memory usage of this object without the object itself. Should
  // be used when embedded inside another object.
  size_t memory_footprint_excluding_this() const;

  // Returns the memory usage of this object including the object itself.
  // Should be used when allocated on the heap.
  size_t memory_footprint_including_this() const;

 private:
  struct padded_lock {
    rw_spinlock lock;
    char padding[CACHELINE_SIZE - (sizeof(rw_spinlock) % CACHELINE_SIZE)];
  };

  int n_cpus_;
  padded_lock *locks_;
};

// Simple implementation of the std::shared_lock API, which is not available in
// the standard library until C++14. Defers error checking to the underlying
// mutex.

template <typename Mutex>
class shared_lock {
 public:
  shared_lock()
      : m_(nullptr) {
  }

  explicit shared_lock(Mutex& m)
      : m_(&m) {
    m_->lock_shared();
  }

  shared_lock(Mutex& m, std::try_to_lock_t t)
      : m_(nullptr) {
    if (m.try_lock_shared()) {
      m_ = &m;
    }
  }

  bool owns_lock() const {
    return m_;
  }

  void swap(shared_lock& other) {
    std::swap(m_,other.m_);
  }

  ~shared_lock() {
    if (m_ != nullptr) {
      m_->unlock_shared();
    }
  }

 private:
  Mutex* m_;
  DISALLOW_COPY_AND_ASSIGN(shared_lock<Mutex>);
};

} // namespace base
#endif
