#include "CorsMiddleware.h"

CorsMiddleware::CorsMiddleware(const CorsConfig& config) : config_(config) {}

void CorsMiddleware::before(HttpRequest& request) {
  LOG_DEBUG << "Handling request in CORS middleware";
  if (request.method() == HttpRequest::OPTIONS) {
    // 只有在OPTIONS请求时才处理预检请求
    LOG_INFO << "Preflight request detected";
    HttpResponse response;
    handle_preflight_request(request, response);
    throw response;  // 抛出响应（CORS预检请求），不需要再往下继续处理请求了
  }
}

void CorsMiddleware::after(HttpResponse& response) {
  LOG_DEBUG << "Handling response in CORS middleware";
  std::vector<std::string> allowed_origins = config_.allowed_origins();
  if (!allowed_origins.empty()) {
    if (std::find(allowed_origins.begin(), allowed_origins.end(), "*") !=
        allowed_origins.end()) {
      // 如果允许所有源，则将响应头中允许的源设为"*"，表示允许访问所有源
      add_cors_headers(response, "*");
    } else {
      // 否则添加第一个允许的源
      add_cors_headers(response, allowed_origins[0]);
    }
  }
}

bool CorsMiddleware::is_origin_allowed(const std::string& origin) const {
  auto allowed_origins = config_.allowed_origins();
  return allowed_origins.empty() ||
         std::find(allowed_origins.begin(), allowed_origins.end(), "*") !=
             allowed_origins.end() ||
         std::find(allowed_origins.begin(), allowed_origins.end(), origin) !=
             allowed_origins.end();
}

void CorsMiddleware::handle_preflight_request(const HttpRequest& request,
                                              HttpResponse& response) {
  // 获取请求的源
  const std::string& origin = request.headers("Origin");
  if (!is_origin_allowed(origin)) {
    // 如果请求的源不被允许访问，返回403 Forbidden
    LOG_WARN << "Origin not allowed: " << origin;
    response.status_code(HttpResponse::FORBIDDEN);
    return;
  }

  // 请求的源被允许，将该源添加到响应头中
  add_cors_headers(response, origin);
  response.status_code(HttpResponse::NO_CONTENT);
  LOG_INFO << "Preflight request handled successfully";
}

void CorsMiddleware::add_cors_headers(HttpResponse& response,
                                      const std::string& origin) {
  try {
    response.header("Access-Control-Allow-Origin", origin);
    if (!config_.allowed_methods().empty()) {
      // 将允许的方法添加到响应头中
      response.header("Access-Control-Allow-Methods",
                      join(config_.allowed_methods(), ", "));
    }

    // 将预检请求的缓存时间（秒）添加到响应头中
    response.header("Access-Control-Max-Age",
                    std::to_string(config_.max_age()));

    LOG_INFO << "CORS headers added successfully";
  } catch (const std::exception& e) {
    LOG_ERROR << "Error adding CORS headers: " << e.what();
  }
}

std::string CorsMiddleware::join(const std::vector<std::string>& strings,
                                 const std::string& delimiter) {
  std::ostringstream oss;
  for (size_t i = 0; i < strings.size(); ++i) {
    if (i > 0) {
      oss << delimiter;
    }
    oss << strings[i];
  }
  return oss.str();
}
