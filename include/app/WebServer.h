#pragma once

#include "AIGame.h"
#include "CorsMiddleware.h"
#include "DatabaseService.h"
#include "EntryHandler.h"
#include "HttpServer.h"
#include "LoginHandler.h"
#include "MenuHandler.h"
#include "StartAIGameHandler.h"

class WebServer : public std::enable_shared_from_this<WebServer> {
 public:
  // 构造函数：传入端口号、服务名称
  WebServer(int port, const std::string &serverName);

  void setThreadNum(int numThreads);

  void start();

  std::shared_ptr<Session> getSession(const HttpRequest &request,
                                      HttpResponse &response);

  void setUserOnline(int userId);
  bool isUserIsOnline(int userId);

  void packageResponse(const HttpRequest &request, HttpResponse &response,
                       HttpResponse::STATUS_CODE statusCode,
                       const std::string &text, bool close,
                       const std::string &contentType, const std::string &body);

  bool hasAIGame(int userId);
  void createAIGame(int userId);

 private:
  // 初始化服务器
  void initialize();

  // 初始化会话
  void initializeSession();

  // 初始化路由：添加路由规则
  void initializeRouter();

  // 初始化中间件：添加中间件
  void initializeMiddleware();

  // 设置会话管理器
  void setSessionManager(std::unique_ptr<SessionManager> sessionManager);

 private:
  bool initialized_;
  std::unique_ptr<HttpServer> httpServer_;
  std::mutex onlineUsersMutex_;
  std::unordered_map<int, bool> onlineUsers_;
  std::mutex AIGamesMutex_;
  std::unordered_map<int, std::shared_ptr<AIGame>> AIGames_;
};