#include "Router.h"

void Router::register_static_route(HttpRequest::METHOD method,
                                   const std::string& path,
                                   handler_ptr handler) {
  handlers_[method][path] = std::move(handler);
}

void Router::register_static_route(HttpRequest::METHOD method,
                                   const std::string& path,
                                   handler_callback callback) {
  callbacks_[method][path] = std::move(callback);
}

void Router::register_dynamic_route(HttpRequest::METHOD method,
                                    const std::string& path,
                                    handler_ptr handler) {
  std::regex path_regex = convert_to_regex(path);
  regex_handlers_.emplace_back(method, path_regex, std::move(handler));
}

void Router::register_dynamic_route(HttpRequest::METHOD method,
                                    const std::string& path,
                                    handler_callback callback) {
  std::regex path_regex = convert_to_regex(path);
  regex_callbacks_.emplace_back(method, path_regex, std::move(callback));
}

bool Router::route(const HttpRequest& request, HttpResponse& response) {
  auto handler_method_it = handlers_.find(request.method());
  if (handler_method_it != handlers_.end()) {
    auto handler_path_it = handler_method_it->second.find(request.path());
    if (handler_path_it != handler_method_it->second.end()) {
      handler_path_it->second->hanle(request, response);
      return true;
    }
  }

  auto callback_method_it = callbacks_.find(request.method());
  if (callback_method_it != callbacks_.end()) {
    auto callback_path_it = callback_method_it->second.find(request.path());
    if (callback_path_it != callback_method_it->second.end()) {
      callback_path_it->second(request, response);
      return true;
    }
  }

  for (const auto& [method, path_regex, handler] : regex_handlers_) {
    std::smatch match;
    std::string path_str = request.path();
    if (method == request.method() &&
        std::regex_match(path_str, match, path_regex)) {
      HttpRequest new_request(request);
      extract_path_parameters(match, new_request);
      handler->hanle(new_request, response);
      return true;
    }
  }

  for (const auto& [method, path_regex, callback] : regex_callbacks_) {
    std::smatch match;
    std::string path_str = request.path();
    if (method == request.method() &&
        std::regex_match(path_str, match, path_regex)) {
      HttpRequest new_request(request);
      extract_path_parameters(match, new_request);
      callback(new_request, response);
      return true;
    }
  }
  return false;  // 没有找到匹配的路由
}

std::regex Router::convert_to_regex(const std::string& path_pattern) {
  std::string regex_pattern =
      "^" +
      std::regex_replace(path_pattern, std::regex(R"(/:([^/]+))"),
                         R"(/([^/]+))") +
      "$";
  return std::regex(regex_pattern);
}

void Router::extract_path_parameters(const std::smatch& match,
                                     HttpRequest& request) {
  for (size_t i = 1; i < match.size(); ++i) {
    request.path_parameters("param" + std::to_string(i), match[i].str());
  }
}
