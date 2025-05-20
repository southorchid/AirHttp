#include "WebServer.h"

WebServer::WebServer(int port, const std::string& name,
                     muduo::net::TcpServer::Option option)
    : httpServer_(port, name, false) {
  initialize();
}

void WebServer::setThreadNum(int numThreads) {
  httpServer_.setThreadNum(numThreads);
}

void WebServer::start() { httpServer_.start(); }

void WebServer::initialize() {
  initializeSession();
  initializeSession();
  initializeMiddleware();
  initializeRouter();
}

void WebServer::initializeSession() {
  auto sessionStorage = std::make_unique<MemorySessionStorage>();
  auto sessionManager =
      std::make_unique<SessionManager>(std::move(sessionStorage));
  setSessionManager(std::move(sessionManager));
}

void WebServer::initializeRouter() {
  httpServer_.add_static_route(HttpRequest::METHOD::GET, "/",
                               std::make_shared<EntryHandler>(this));
}

void WebServer::initializeMiddleware() {
  auto corsMiddleware = std::make_shared<CorsMiddleware>();
  httpServer_.add_middleware(corsMiddleware);
}

void WebServer::setSessionManager(
    std::unique_ptr<SessionManager> sessionManager) {
  httpServer_.session_manager(std::move(sessionManager));
}
