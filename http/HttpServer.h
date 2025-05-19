#pragma once

#include <muduo/base/Logging.h>
#include <muduo/net/EventLoop.h>
#include <muduo/net/TcpServer.h>

#include "HttpContext.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "Router.h"

class HttpServer : private muduo::noncopyable {
 public:
  using Function = std::function<void(const HttpRequest &, HttpResponse &)>;

  // 新连接建立时回调
  void onconnection(const muduo::net::TcpConnectionPtr &conn);

  // 接收连接数据的消息回调
  void onmessage(const muduo::net::TcpConnectionPtr &conn,
                 muduo::net::Buffer *buf, muduo::Timestamp receive_time);

  // 当接收到消息回调后调用的处理
  void onrequest(const muduo::net::TcpConnectionPtr &conn,
                 const HttpRequest &request);

  // 执行对应的路由处理函数
  void onhttpcallback(const HttpRequest &request, HttpResponse &response);

 private:
  muduo::net::InetAddress address_;
  muduo::net::TcpServer server_;
  muduo::net::EventLoop main_loop_;
  Function callback_;
  Router router_;
};