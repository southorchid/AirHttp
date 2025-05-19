#include "MiddlewareChain.h"

#include "CorsMiddleware.h"

void MiddlewareChain::add_middleware(std::shared_ptr<Middleware> middleware) {
  middlewares_.emplace_back(middleware);
}

void MiddlewareChain::before(HttpRequest& request) {
  for (const auto& middleware : middlewares_) {
    middleware->before(request);
  }
}

void MiddlewareChain::after(HttpResponse& response) {
  try {
    for (auto it = middlewares_.rbegin(); it != middlewares_.rend(); ++it) {
      if (*it) {
        (*it)->after(response);
      }
    }
  } catch (const std::exception& e) {
    LOG_ERROR << "Error in middleware chain: " << e.what();
  }
}
