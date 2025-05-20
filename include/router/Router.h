#pragma once

#include <regex>
#include <string>
#include <unordered_map>
#include <vector>

#include "HttpRequest.h"
#include "HttpResponse.h"
#include "RouterHandler.h"

using handler_ptr = std::shared_ptr<RouterHandler>;
using handler_callback = std::function<void(const HttpRequest&, HttpResponse&)>;

struct RouteHandler {
  RouteHandler(HttpRequest::METHOD method, std::regex path_regex,
               handler_ptr handler)
      : method_(method), path_regex_(path_regex), handler_(handler) {}
  HttpRequest::METHOD method_;
  std::regex path_regex_;
  handler_ptr handler_;
};

struct RouteCallback {
  RouteCallback(HttpRequest::METHOD method, std::regex path_regex,
                handler_callback callback)
      : method_(method), path_regex_(path_regex), callback_(callback) {}
  HttpRequest::METHOD method_;
  std::regex path_regex_;
  handler_callback callback_;
};

class Router {
 public:
  // 注册对象式的静态路由，适合复杂的处理逻辑
  void register_static_route(HttpRequest::METHOD method,
                             const std::string& path, handler_ptr handler);

  // 注册函数式的静态路由，适合简单的处理逻辑
  void register_static_route(HttpRequest::METHOD method,
                             const std::string& path,
                             handler_callback callback);

  // 注册对象式的动态路由，适合复杂的处理逻辑
  void register_dynamic_route(HttpRequest::METHOD method,
                              const std::string& path, handler_ptr handler);

  // 注册函数式的动态路由，适合简单的处理逻辑
  void register_dynamic_route(HttpRequest::METHOD method,
                              const std::string& path,
                              handler_callback callback);

  // 查找并执行路由
  bool route(const HttpRequest& request, HttpResponse& response);

  // 将路径模式转换为正则表达式
  std::regex convert_to_regex(const std::string& path);

  // 提取路径参数
  void extract_path_parameters(const std::smatch& match, HttpRequest& request);

 private:
  std::unordered_map<HttpRequest::METHOD,
                     std::unordered_map<std::string, handler_ptr>>
      handlers_;
  std::unordered_map<HttpRequest::METHOD,
                     std::unordered_map<std::string, handler_callback>>
      callbacks_;
  std::vector<RouteHandler> regex_handlers_;
  std::vector<RouteCallback> regex_callbacks_;
};