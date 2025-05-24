#include "WebServer.h"

WebServer::WebServer(int port, const std::string& name)
    : initialized_(false),
      httpServer_(std::make_unique<HttpServer>(port, name)) {}

void WebServer::setThreadNum(int numThreads) {
  httpServer_->setThreadNum(numThreads);
}

void WebServer::start() {
  // 初始化服务器
  if (!initialized_) {
    this->initialize();
  }
  httpServer_->start();
}

std::shared_ptr<Session> WebServer::getSession(const HttpRequest& request,
                                               HttpResponse& response) {
  return httpServer_->session_manager()->get_session(request, response);
}

void WebServer::setUserOnline(int userId) {
  std::lock_guard<std::mutex> lock(onlineUsersMutex_);
  onlineUsers_[userId] = true;
}

bool WebServer::isUserIsOnline(int userId) {
  std::lock_guard<std::mutex> lock(onlineUsersMutex_);
  if (onlineUsers_.find(userId) == onlineUsers_.end()) {
    return false;
  }
  return onlineUsers_[userId];
}

void WebServer::packageResponse(const HttpRequest& request,
                                HttpResponse& response,
                                HttpResponse::STATUS_CODE statusCode,
                                const std::string& text, bool close,
                                const std::string& contentType,
                                const std::string& body) {
  try {
    response.version(request.version());
    response.status_code(statusCode);
    response.text(text);
    response.close_connection(close);
    response.content_type(contentType);
    response.content_length(body.size());
    response.body(body);
  } catch (std::exception& e) {
    LOG_ERROR << "Failed to package response: " << e.what();
    response.version("HTTP/1.1");
    response.status_code(HttpResponse::INTERNAL_SERVER_ERROR);
    response.text("Internal Server Error");
    response.close_connection(true);
  }
}

bool WebServer::hasAIGame(int userId) {
  std::lock_guard<std::mutex> lock(AIGamesMutex_);
  return AIGames_.find(userId) != AIGames_.end();
}

void WebServer::createAIGame(int userId) {
  std::lock_guard<std::mutex> lock(AIGamesMutex_);
  AIGames_[userId] = std::make_shared<AIGame>();
}

void WebServer::initialize() {
  DatabaseConnectionPool::getInstance().init("127.0.0.1", "root", "617020@Dai",
                                             "Gomoku", 10);
  // 初始化会话
  this->initializeSession();
  // 初始化中间件
  this->initializeMiddleware();
  // 初始化路由
  this->initializeRouter();
  initialized_ = true;
}

void WebServer::initializeSession() {
  try {
    auto sessionStorage = std::make_unique<MemorySessionStorage>();
    auto sessionManager =
        std::make_unique<SessionManager>(std::move(sessionStorage));
    this->setSessionManager(std::move(sessionManager));
  } catch (std::exception& e) {
    LOG_ERROR << "Failed to initialize session: " << e.what();
    throw std::runtime_error("Failed to initialize session");
  }
}

void WebServer::initializeRouter() {
  // 注册入口路由
  httpServer_->add_static_route(
      HttpRequest::METHOD::GET, "/",
      std::make_shared<EntryHandler>(shared_from_this()));
  httpServer_->add_static_route(
      HttpRequest::METHOD::GET, "/entry",
      std::make_shared<EntryHandler>(shared_from_this()));
  // 注册登录路由
  httpServer_->add_static_route(
      HttpRequest::METHOD::POST, "/login",
      std::make_shared<LoginHandler>(shared_from_this()));
  // 注册菜单路由
  httpServer_->add_static_route(
      HttpRequest::METHOD::GET, "/menu",
      std::make_shared<MenuHandler>(shared_from_this()));
  // 注册人机对战路由
  httpServer_->add_static_route(
      HttpRequest::METHOD::GET, "/aiBot/start",
      std::make_shared<StartAIGameHandler>(shared_from_this()));
}

void WebServer::initializeMiddleware() {
  // 添加跨域中间件
  auto corsMiddleware = std::make_shared<CorsMiddleware>();
  httpServer_->add_middleware(corsMiddleware);
}

void WebServer::setSessionManager(
    std::unique_ptr<SessionManager> sessionManager) {
  httpServer_->session_manager(std::move(sessionManager));
}
