#pragma once

#include <string>

#include "SslTypes.h"

class SslConfig {
 public:
  SslConfig();

  // 证书相关配置
  void certificate_file(const std::string& cert_file);
  const std::string& certificate_file() const;
  void private_key_file(const std::string& key_file);
  const std::string& private_key_file() const;
  void certificate_chain_file(const std::string& chain_file);
  const std::string& certificate_chain_file() const;

  // SSL协议版本和加密套件相关配置
  void protocol_version(SSLVersion version);
  SSLVersion protocol_version() const;
  void cipher_list(const std::string& cipher_list);
  const std::string& cipher_list() const;

  // 客户端认证相关配置
  void verify_client(bool verify);
  bool verify_client() const;
  void verify_depth(int depth);
  int verify_depth() const;

  // 会话相关配置
  void session_timeout(int seconds);
  int session_timeout() const;
  void session_cache_size(long size);
  long session_cache_size() const;

 private:
  std::string cert_file_;    // 证书文件
  std::string key_file_;     // 私钥文件
  std::string chain_file_;   // 证书链文件
  SSLVersion version_;       // SSL协议版本
  std::string cipher_list_;  // 加密套件
  bool verify_client_;       // 是否验证客户端
  int verify_depth_;         // 客户端认证的最大深度
  int session_timeout_;      // 会话超时时间
  long session_cache_size_;  // 会话缓存大小
};