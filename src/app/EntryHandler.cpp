#include "EntryHandler.h"

EntryHandler::EntryHandler(WebServer* webServer) : webServer_(webServer) {}

void EntryHandler::handle(const HttpRequest& request, HttpResponse& response) {
  response.status_code(HttpResponse::OK);
  response.close_connection(false);
  response.content_type("text/plain");
  response.content_length(13);
  response.body("Hello, world!");
}
