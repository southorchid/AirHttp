#include "LoginHandler.h"

LoginHandler::LoginHandler(std::shared_ptr<WebServer> server)
    : server_(server) {}

void LoginHandler::handle(const HttpRequest& request, HttpResponse& response) {
  try {
    LOG_DEBUG << "LoginHandler::handle";
    // 验证content-type
    auto contentType = request.header("Content-Type");
    if (contentType.empty() || contentType != "application/json" ||
        request.body().empty()) {
      response.response_line(request.version(), HttpResponse::BAD_REQUEST,
                             "Bad Request");
      response.close_connection(true);
      response.content_length(0);
      return;
    }

    // 解析json
    json parsed = json::parse(request.body());
    std::string username = parsed["username"];
    std::string password = parsed["password"];

    // 验证用户名和密码
    int userId = -1;
    switch (DatabaseService::login(username, password, userId)) {
      case DatabaseService::LoginResult::SUCCESS: {
        auto session = server_->getSession(request, response);
        session->value("userId", std::to_string(userId));
        session->value("username", username);
        session->value("isLogin", "true");
        if (!server_->isUserIsOnline(userId)) {
          server_->setUserOnline(userId);
          json successResponse;
          successResponse["success"] = true;
          successResponse["userId"] = userId;
          std::string successBody = successResponse.dump();
          response.response_line(request.version(), HttpResponse::OK, "OK");
          response.close_connection(false);
          response.content_type("application/json");
          response.content_length(successBody.size());
          response.body(successBody);
        } else {
          // 用户已经在线
          json failureResponse;
          failureResponse["success"] = false;
          failureResponse["error"] = "User is already online";
          std::string failureBody = failureResponse.dump();
          response.response_line(request.version(), HttpResponse::BAD_REQUEST,
                                 "Bad Request");
          response.close_connection(true);
          response.content_type("application/json");
          response.content_length(failureBody.size());
          response.body(failureBody);
        }
        break;
      }
      case DatabaseService::LoginResult::USER_NOT_REGISTERED: {
        json failureResponse;
        failureResponse["success"] = false;
        failureResponse["error"] = "User not registered";
        std::string failureBody = failureResponse.dump();
        response.response_line(request.version(), HttpResponse::BAD_REQUEST,
                               "Bad Request");
        response.close_connection(true);
        response.content_type("application/json");
        response.content_length(failureBody.size());
        response.body(failureBody);
        break;
      }
      case DatabaseService::LoginResult::PASSWORD_INCORRECT: {
        json failureResponse;
        failureResponse["success"] = false;
        failureResponse["error"] = "Password incorrect";
        std::string failureBody = failureResponse.dump();
        response.response_line(request.version(), HttpResponse::BAD_REQUEST,
                               "Bad Request");
        response.close_connection(true);
        response.content_type("application/json");
        response.content_length(failureBody.size());
        response.body(failureBody);
        break;
      }
      case DatabaseService::LoginResult::SERVER_ERROR: {
        json failureResponse;
        failureResponse["success"] = false;
        failureResponse["error"] = "Server error";
        std::string failureBody = failureResponse.dump();
        response.response_line(request.version(),
                               HttpResponse::INTERNAL_SERVER_ERROR,
                               "Internal Server Error");
        response.close_connection(true);
        response.content_type("application/json");
        response.content_length(failureBody.size());
        response.body(failureBody);
        break;
      }
      default: {
        json failureResponse;
        failureResponse["success"] = false;
        failureResponse["error"] = "Unknown error";
        std::string failureBody = failureResponse.dump();
        response.response_line(request.version(),
                               HttpResponse::INTERNAL_SERVER_ERROR,
                               "Internal Server Error");
        response.close_connection(true);
        response.content_type("application/json");
        response.content_length(failureBody.size());
        response.body(failureBody);
        break;
      }
    }
  } catch (const std::exception& e) {
    LOG_DEBUG << "LoginHandler::handle exception: " << e.what();
    response.response_line(request.version(),
                           HttpResponse::INTERNAL_SERVER_ERROR,
                           "Internal Server Error");
    response.close_connection(true);
    response.content_length(0);
  }
}