#ifndef BASE_NET_DNS_RESOLVER_H
#define BASE_NET_DNS_RESOLVER_H

#include <vector>

#include "base/core/gscoped_ptr.h"
#include "base/core/macros.h"
#include "base/util/synchronizer.h"
#include "base/util/status.h"

namespace base {

class HostPort;
class Sockaddr;
class ThreadPool;

// DNS Resolver which supports async address resolution.
class DnsResolver {
 public:
  DnsResolver();
  ~DnsResolver();

  // Resolve any addresses corresponding to this host:port pair.
  // Note that a host may resolve to more than one IP address.
  //
  // 'addresses' may be NULL, in which case this function simply checks that
  // the host/port pair can be resolved, without returning anything.
  //
  // When the result is available, or an error occurred, 'cb' is called
  // with the result Status.
  //
  // NOTE: the callback should be fast since it is called by the DNS
  // resolution thread.
  // NOTE: in some rare cases, the callback may also be called inline
  // from this function call, on the caller's thread.
  void ResolveAddresses(const HostPort& hostport,
                        std::vector<Sockaddr>* addresses,
                        const StatusCallback& cb);

 private:
  gscoped_ptr<ThreadPool> pool_;

  DISALLOW_COPY_AND_ASSIGN(DnsResolver);
};

} // namespace base
#endif /* BASE_NET_DNS_RESOLVER_H */
