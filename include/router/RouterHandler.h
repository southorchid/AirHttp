#pragma once

#include "HttpRequest.h"
#include "HttpResponse.h"

class RouterHandler {
 public:
  virtual void handle(const HttpRequest& request, HttpResponse& response) = 0;
};