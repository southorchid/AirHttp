#pragma once

#include <muduo/base/Logging.h>

#include <algorithm>

#include "CorsConfig.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "Middleware.h"

// 跨域资源共享（CORS）中间件类，负责处理跨域请求和响应
class CorsMiddleware : public Middleware {
 public:
  explicit CorsMiddleware(
      const CorsConfig& config = CorsConfig::default_config());

  void before(HttpRequest& request) override;

  void after(HttpResponse& response) override;

  // 检查请求的源是否被允许
  bool is_origin_allowed(const std::string& origin) const;

  // 处理预检请求
  void handle_preflight_request(const HttpRequest& request,
                                HttpResponse& response);

  // 添加CORS相关的响应头
  void add_cors_headers(HttpResponse& response, const std::string& origin);

  // 工具函数：将字符串数组序列化（以delimiter为分隔符）
  std::string join(const std::vector<std::string>& vec,
                   const std::string& delimiter);

 private:
  CorsConfig config_;           // CORS配置
  const HttpRequest* request_;  // 当前请求指针
};