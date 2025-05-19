#pragma once

#include "HttpRequest.h"
#include "HttpResponse.h"

class RouterHandler {
 public:
  virtual void hanle(const HttpRequest& request, HttpResponse& response) = 0;
};