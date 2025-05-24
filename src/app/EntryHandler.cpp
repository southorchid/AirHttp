#include "EntryHandler.h"

#include "LoginHandler.h"

EntryHandler::EntryHandler(std::shared_ptr<WebServer> server)
    : server_(server) {}

void EntryHandler::handle(const HttpRequest& request, HttpResponse& response) {
  std::string file_path("./static/entry.html");
  std::string content;
  if (FileManager::readFile(file_path, content) == -1) {
    FileManager::readFile("./static/NotFound.html", content);
    response.response_line(request.version(), HttpResponse::NOT_FOUND,
                           "Not Found");
    response.header("Content-Type", "text/html");
    response.content_length(content.size());
    response.body(content);
    return;
  }

  response.response_line(request.version(), HttpResponse::OK, "OK");
  response.header("Content-Type", "text/html");
  response.content_length(content.size());
  response.body(content);
}
