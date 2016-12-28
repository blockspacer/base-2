#ifndef BASE_HTTP_WEBSERVER_OPTIONS_H
#define BASE_HTTP_WEBSERVER_OPTIONS_H

#include <string>
#include <stdint.h>

namespace base {

// Options controlling the web server.
// The default constructor sets these from the gflags defined in webserver_options.cc.
// See those flags for documentation.
struct WebserverOptions {
  WebserverOptions();

  std::string bind_interface;
  uint16_t port;
  std::string doc_root;
  bool enable_doc_root;
  std::string certificate_file;
  std::string authentication_domain;
  std::string password_file;
  uint32_t num_worker_threads;
};

} // namespace base
#endif /* BASE_HTTP_WEBSERVER_OPTIONS_H */
