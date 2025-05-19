#include "CorsConfig.h"

CorsConfig::CorsConfig() : max_age_(3600) {}

std::vector<std::string> CorsConfig::allowed_origins() const {
  return allowed_origins_;
}

std::vector<std::string> CorsConfig::allowed_methods() const {
  return allowed_methods_;
}

int CorsConfig::max_age() const { return max_age_; }

CorsConfig CorsConfig::default_config() {
  CorsConfig config;
  // 允许所有源
  config.allowed_origins_ = {"*"};
  // 允许的HTTP方法
  config.allowed_methods_ = {"GET", "POST", "PUT", "DELETE", "OPTIONS"};
  // 允许的HTTP请求头
  config.allowed_headers_ = {"Content-Type", "Authorization"};
  // 允许携带凭证
  config.allowed_credentials_ = true;
  // 预检请求的缓存时间为3600秒
  config.max_age_ = 3600;
  return config;
}
