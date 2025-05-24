#pragma once

#include <nlohmann/json.hpp>

#include "FileManager.h"
#include "WebServer.h"

class MenuHandler : public RouterHandler {
 public:
  using json = nlohmann::json;
  explicit MenuHandler(std::shared_ptr<WebServer> server);

  void handle(const HttpRequest& request, HttpResponse& response) override;

 private:
  std::shared_ptr<WebServer> server_;
};