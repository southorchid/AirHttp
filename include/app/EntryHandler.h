#pragma once

#include "RouterHandler.h"
#include "WebServer.h"

class WebServer;
class EntryHandler : public RouterHandler {
 public:
  explicit EntryHandler(WebServer* webServer);

  void handle(const HttpRequest& request, HttpResponse& response) override;

 private:
  WebServer* webServer_;
};