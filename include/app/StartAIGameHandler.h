#pragma once

#include <nlohmann/json.hpp>

#include "FileManager.h"
#include "WebServer.h"

class StartAIGameHandler : public RouterHandler {
 public:
  using json = nlohmann::json;
  explicit StartAIGameHandler(std::shared_ptr<WebServer> server);

  void handle(const HttpRequest& request, HttpResponse& response) override;

 private:
  std::shared_ptr<WebServer> server_;
};