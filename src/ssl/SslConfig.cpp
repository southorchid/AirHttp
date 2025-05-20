#include "SslConfig.h"

SslConfig::SslConfig()
    : version_(SSLVersion::TLS_1_2),
      cipher_list_("HIGH:!aNULL:!MD5"),
      verify_client_(false),
      verify_depth_(4),
      session_timeout_(300),
      session_cache_size_(20480L) {}

void SslConfig::certificate_file(const std::string& cert_file) {
  cert_file_ = cert_file;
}

const std::string& SslConfig::certificate_file() const { return cert_file_; }

void SslConfig::private_key_file(const std::string& key_file) {
  key_file_ = key_file;
}

const std::string& SslConfig::private_key_file() const { return key_file_; }

void SslConfig::certificate_chain_file(const std::string& chain_file) {
  chain_file_ = chain_file;
}

const std::string& SslConfig::certificate_chain_file() const {
  return chain_file_;
}

void SslConfig::protocol_version(SSLVersion version) { version_ = version; }

SSLVersion SslConfig::protocol_version() const { return version_; }

void SslConfig::cipher_list(const std::string& cipher_list) {
  cipher_list_ = cipher_list;
}

const std::string& SslConfig::cipher_list() const { return cipher_list_; }

void SslConfig::verify_client(bool verify) { verify_client_ = verify; }

bool SslConfig::verify_client() const { return verify_client_; }

void SslConfig::verify_depth(int depth) { verify_depth_ = depth; }

int SslConfig::verify_depth() const { return verify_depth_; }

void SslConfig::session_timeout(int seconds) { session_timeout_ = seconds; }

int SslConfig::session_timeout() const { return session_timeout_; }

void SslConfig::session_cache_size(long size) { session_cache_size_ = size; }

long SslConfig::session_cache_size() const { return session_cache_size_; }
