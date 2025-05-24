#include "MenuHandler.h"

MenuHandler::MenuHandler(std::shared_ptr<WebServer> server) : server_(server) {}

void MenuHandler::handle(const HttpRequest& request, HttpResponse& response) {
  try {
    // 检查是否登录
    auto session = server_->getSession(request, response);
    if (session->value("isLogin") != "true") {
      json unauthorizedResponse;
      unauthorizedResponse["status"] = "error";
      unauthorizedResponse["message"] = "Unauthorized";
      std::string unauthorizedBody = unauthorizedResponse.dump();
      server_->packageResponse(
          request, response, HttpResponse::STATUS_CODE::UNAUTHORIZED,
          "Unauthorized", true, "application/json", unauthorizedBody);
      return;
    }

    int userId = std::stoi(session->value("userId"));
    std::string username = session->value("username");

    std::string menuHtml;
    FileManager::readFile("./static/menu.html", menuHtml);

    size_t headEnd = menuHtml.find("</head>");
    if (headEnd != std::string::npos) {
      std::string script =
          "<script>const userId = '" + std::to_string(userId) + "';</script>";
      menuHtml.insert(headEnd, script);
    }

    server_->packageResponse(request, response, HttpResponse::STATUS_CODE::OK,
                             "OK", false, "text/html", menuHtml);
  } catch (std::exception& e) {
    server_->packageResponse(
        request, response, HttpResponse::STATUS_CODE::INTERNAL_SERVER_ERROR,
        "Internal Server Error", true, "text/plain", e.what());
  }
}