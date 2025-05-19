#pragma once
#include <muduo/base/Logging.h>

#include <memory>
#include <vector>

#include "Middleware.h"

class MiddlewareChain {
 public:
  // 添加中间件
  void add_middleware(std::shared_ptr<Middleware> middleware);

  // 执行处理请求前的中间件链
  void before(HttpRequest &request);

  // 执行处理响应后的中间件链
  void after(HttpResponse &response);

 private:
  std::vector<std::shared_ptr<Middleware>> middlewares_;
};