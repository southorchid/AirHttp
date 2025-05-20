#pragma once

#include <muduo/base/Logging.h>
#include <muduo/base/noncopyable.h>
#include <openssl/err.h>
#include <openssl/ssl.h>

#include "SslConfig.h"

class SslContext : private muduo::noncopyable {
 public:
  explicit SslContext(const SslConfig& config);
  ~SslContext();

  bool initialize();
  SSL_CTX* get_native_handle() const;

  bool load_certificates();
  bool setup_protocol();
  void setup_session_cache();
  static void handle_ssl_error(const char* message);

 private:
  SSL_CTX* ctx_;      // SSL上下文
  SslConfig config_;  // SSL配置
};