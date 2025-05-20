#pragma once

#include "CorsMiddleware.h"
#include "DatabaseService.h"
#include "EntryHandler.h"
#include "HttpServer.h"

class WebServer {
 public:
  WebServer(int port, const std::string &name,
            muduo::net::TcpServer::Option option =
                muduo::net::TcpServer::kNoReusePort);

  void setThreadNum(int numThreads);

  void start();

 private:
  void initialize();

  void initializeSession();

  void initializeRouter();

  void initializeMiddleware();

  void setSessionManager(std::unique_ptr<SessionManager> sessionManager);

 private:
  HttpServer httpServer_;
};