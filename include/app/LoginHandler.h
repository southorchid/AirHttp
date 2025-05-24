#pragma once

#include <nlohmann/json.hpp>

#include "DatabaseService.h"
#include "WebServer.h"

class WebServer;
class LoginHandler : public RouterHandler {
 public:
  using json = nlohmann::json;

  explicit LoginHandler(std::shared_ptr<WebServer> server);

  // 处理登录逻辑
  void handle(const HttpRequest& request, HttpResponse& response) override;

 private:
  std::shared_ptr<WebServer> server_;
};