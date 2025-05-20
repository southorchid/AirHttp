#pragma once

#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>

#include "HttpContext.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "MiddlewareChain.h"
#include "Router.h"
#include "SessionManager.h"
#include "SslConnection.h"
#include "SslContext.h"

class HttpServer : private muduo::noncopyable {
 public:
  using http_callback_t =
      std::function<void(const HttpRequest &, HttpResponse &)>;

  // 构造函数
  HttpServer(int port, const std::string &name, bool useSSL = false,
             muduo::net::TcpServer::Option option =
                 muduo::net::TcpServer::kNoReusePort);

  void setThreadNum(int numThreads);

  void start();

  muduo::net::EventLoop *get_loop();

  void http_callback(const http_callback_t &cb);

  void add_static_route(HttpRequest::METHOD method, const std::string &path,
                        const http_callback_t &cb);

  void add_static_route(HttpRequest::METHOD method, const std::string &path,
                        handler_ptr handler);

  void add_dynamic_route(HttpRequest::METHOD method, const std::string &path,
                         const http_callback_t &cb);

  void add_dynamic_route(HttpRequest::METHOD method, const std::string &path,
                         handler_ptr handler);

  void session_manager(std::unique_ptr<SessionManager> manager);
  SessionManager *session_manager() const;

  void add_middleware(std::shared_ptr<Middleware> middleware);

  void enable_ssl(bool enable);

  void ssl_config(const SslConfig &config);

 private:
  void initialize();

  // 新连接建立时回调
  void onConnection(const muduo::net::TcpConnectionPtr &conn);

  // 接收连接数据的消息回调
  void onMessage(const muduo::net::TcpConnectionPtr &conn,
                 muduo::net::Buffer *buf, muduo::Timestamp receive_time);

  // 当接收到消息回调后调用的处理
  void onRequest(const muduo::net::TcpConnectionPtr &conn,
                 const HttpRequest &request);

  // 执行对应的路由处理函数
  void handle_request(const HttpRequest &request, HttpResponse &response);

 private:
  muduo::net::InetAddress address_;                  // 监听地址
  muduo::net::TcpServer server_;                     // TCP服务器
  muduo::net::EventLoop main_loop_;                  // 主事件循环
  http_callback_t http_callback_;                    // 回调函数
  Router router_;                                    // 路由
  std::unique_ptr<SessionManager> session_manager_;  // 会话管理器
  MiddlewareChain middleware_chain_;                 // 中间件链
  bool useSSL_;                                      // 是否使用SSL
  std::unique_ptr<SslContext> ssl_context_;          // SSL上下文
  std::unordered_map<muduo::net::TcpConnectionPtr,
                     std::unique_ptr<SslConnection>>
      ssl_connections_;  // SSL连接映射表
};