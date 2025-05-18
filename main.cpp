#include <iostream>

#include "HttpContext.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "Router.h"

int main() {
  Router router;
  router.register_static_route(
      HttpRequest::GET, "/",
      [](const HttpRequest& request, HttpResponse& response) {
        std::cout << "Hello, World!" << std::endl;
      });
  router.register_dynamic_route(
      HttpRequest::GET, "/user/:id",
      [](const HttpRequest& request, HttpResponse& response) {
        std::cout << "User ID: " << request.path_parameters("param1")
                  << std::endl;
      });
  HttpRequest request;
  request.method("GET");
  request.path("/user/123");
  HttpResponse response(false);
  router.route(request, response);
  return 0;
}