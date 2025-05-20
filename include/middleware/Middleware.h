#pragma once

#include <memory>

#include "HttpRequest.h"
#include "HttpResponse.h"

class Middleware {
 public:
  // 处理请求前的操作
  virtual void before(HttpRequest &request) = 0;
  // 生成响应后的操作
  virtual void after(HttpResponse &response) = 0;
  // 设置下一个中间件
  void next(std::shared_ptr<Middleware> next) { next_middleware_ = next; }

 private:
  std::shared_ptr<Middleware> next_middleware_;
};