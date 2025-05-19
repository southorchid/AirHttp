#include "HttpServer.h"

void HttpServer::onconnection(const muduo::net::TcpConnectionPtr& conn) {
  if (conn->connected()) {
    conn->setContext(HttpContext());  // 设置连接上下文
  }
}

void HttpServer::onmessage(const muduo::net::TcpConnectionPtr& conn,
                           muduo::net::Buffer* buf,
                           muduo::Timestamp receive_time) {
  // 从连接中取出HttpContext
  HttpContext* context =
      boost::any_cast<HttpContext>(conn->getMutableContext());
  try {
    // 解析请求封装到HttpRequest对象中
    if (!context->parse_request(buf, receive_time)) {
      // 解析HTTP报文失败，证明HTTP请求报文格式或语法错误
      conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
      conn->shutdown();
    }

    if (context->parse_request_end()) {
      // 解析到一个完整的HTTP请求报文，处理请求
      onrequest(conn, context->request());
      context->reset();  // 重置上下文
    }
  } catch (const std::exception& e) {
    LOG_ERROR << "Exception in onmessage: " << e.what();
    conn->send("HTTP/1.1 500 Internal Server Error\r\n\r\n");
    conn->shutdown();
  }
}

void HttpServer::onrequest(const muduo::net::TcpConnectionPtr& conn,
                           const HttpRequest& request) {
  const std::string& connection = request.headers("Connection");
  bool close = (connection == "close" || (request.version() == "HTTP/1.0" &&
                                          connection != "Keep-Alive"));
  HttpResponse response(close);

  if (request.method() == HttpRequest::OPTIONS) {
    response.response_line(request.version(), HttpResponse::OK, "OK");
    response.headers("Access-Control-Allow-Origin", "*");
    response.headers("Access-Control-Allow-Methods",
                     "GET, POST, PUT, DELETE, OPTIONS");
    response.headers("Access-Control-Allow-Headers", "Content-Type");
    response.headers("Access-Control-Max-Age", "86400");

    muduo::net::Buffer buf;
    buf.append(response.str());
    conn->send(&buf);
    return;
  }

  callback_(request, response);  // 执行回调函数处理

  muduo::net::Buffer buf;
  buf.append(response.str());
  conn->send(&buf);  // 发送响应

  if (response.close_connection()) {
    conn->shutdown();  // 关闭连接
  }
}

void HttpServer::onhttpcallback(const HttpRequest& request,
                                HttpResponse& response) {
  // 执行路由处理函数
  if (!router_.route(request, response)) {
    response.status_code(HttpResponse::NOT_FOUND);
    response.text("Not Found");
    response.close_connection(true);
  }
}