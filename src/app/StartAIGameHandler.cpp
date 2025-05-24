#include "StartAIGameHandler.h"

StartAIGameHandler::StartAIGameHandler(std::shared_ptr<WebServer> server)
    : server_(server) {}

void StartAIGameHandler::handle(const HttpRequest& request,
                                HttpResponse& response) {
  try {
    auto session = server_->getSession(request, response);
    if (session->value("isLogin") != "true") {
      // 用户未登录
      json unauthorizedBody;
      unauthorizedBody["status"] = "error";
      unauthorizedBody["message"] = "Unauthorized";
      server_->packageResponse(request, response, HttpResponse::UNAUTHORIZED,
                               "Unauthorized", true, "application/json",
                               unauthorizedBody.dump());
      return;
    }

    int userId = std::stoi(session->value("userId"));
    if (!server_->hasAIGame(userId)) {
      server_->createAIGame(userId);
    }

    std::string chessGameVsAIHtml;
    FileManager::readFile("./static/ChessGameVsAi.html", chessGameVsAIHtml);
    server_->packageResponse(request, response, HttpResponse::OK, "OK", false,
                             "text/html", chessGameVsAIHtml);
  } catch (std::exception& e) {
    server_->packageResponse(
        request, response, HttpResponse::INTERNAL_SERVER_ERROR,
        "Internal Server Error", true, "text/plain", e.what());
  }
}