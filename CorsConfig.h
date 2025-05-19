#pragma once

#include <string>
#include <vector>

// 跨域资源共享（CORS）配置类，负责管理和配置跨域资源共享（CORS）策略
class CorsConfig {
 public:
  CorsConfig();

  // 设置允许访问资源的源列表
  std::vector<std::string> allowed_origins() const;

  // 获取允许的HTTP方法列表
  std::vector<std::string> allowed_methods() const;

  // 获取预检请求的缓存时间（秒）
  int max_age() const;

  // 默认配置
  static CorsConfig default_config();

 private:
  std::vector<std::string> allowed_origins_;  // 允许访问资源的源列表
  std::vector<std::string> allowed_methods_;  // 允许的HTTP方法列表
  std::vector<std::string> allowed_headers_;  // 允许的 HTTP 请求头列表
  bool allowed_credentials_;  // 表示是否允许携带凭证（如 cookies、HTTP 认证等）
  int max_age_;               // 预检请求的缓存时间（秒）
};