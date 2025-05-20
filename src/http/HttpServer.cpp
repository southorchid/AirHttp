#include "HttpServer.h"

HttpServer::HttpServer(int port, const std::string& name, bool useSSL,
                       muduo::net::TcpServer::Option option)
    : address_(port),
      server_(&main_loop_, address_, name, option),
      useSSL_(useSSL),
      http_callback_(std::bind(&HttpServer::handle_request, this,
                               std::placeholders::_1, std::placeholders::_2)) {
  initialize();
}

void HttpServer::setThreadNum(int numThreads) {
  server_.setThreadNum(numThreads);
}

void HttpServer::start() {
  LOG_INFO << "HttpServer[" << server_.name() << "] starts listening on "
           << server_.ipPort();
  server_.start();
  main_loop_.loop();
}

muduo::net::EventLoop* HttpServer::get_loop() { return &main_loop_; }

void HttpServer::http_callback(const http_callback_t& cb) {
  http_callback_ = cb;
}

void HttpServer::add_static_route(HttpRequest::METHOD method,
                                  const std::string& path,
                                  const http_callback_t& cb) {
  router_.register_static_route(method, path, cb);
}

void HttpServer::add_static_route(HttpRequest::METHOD method,
                                  const std::string& path,
                                  handler_ptr handler) {
  router_.register_static_route(method, path, handler);
}

void HttpServer::add_dynamic_route(HttpRequest::METHOD method,
                                   const std::string& path,
                                   const http_callback_t& cb) {
  router_.register_dynamic_route(method, path, cb);
}

void HttpServer::add_dynamic_route(HttpRequest::METHOD method,
                                   const std::string& path,
                                   handler_ptr handler) {
  router_.register_dynamic_route(method, path, handler);
}

void HttpServer::session_manager(std::unique_ptr<SessionManager> manager) {
  session_manager_ = std::move(manager);
}

SessionManager* HttpServer::session_manager() const {
  return session_manager_.get();
}

void HttpServer::add_middleware(std::shared_ptr<Middleware> middleware) {
  middleware_chain_.add_middleware(middleware);
}

void HttpServer::enable_ssl(bool enable) { useSSL_ = enable; }

void HttpServer::ssl_config(const SslConfig& config) {
  if (useSSL_) {
    ssl_context_ = std::make_unique<SslContext>(config);
    if (!ssl_context_->initialize()) {
      LOG_ERROR << "Failed to initialize SSL context";
      abort();
    }
  }
}

void HttpServer::initialize() {
  server_.setConnectionCallback(
      std::bind(&HttpServer::onConnection, this, std::placeholders::_1));
  server_.setMessageCallback(
      std::bind(&HttpServer::onMessage, this, std::placeholders::_1,
                std::placeholders::_2, std::placeholders::_3));
  http_callback_ = std::bind(&HttpServer::handle_request, this,
                             std::placeholders::_1, std::placeholders::_2);
}

void HttpServer::onConnection(const muduo::net::TcpConnectionPtr& conn) {
  if (conn->connected()) {
    if (useSSL_) {
      auto ssl_conn = std::make_unique<SslConnection>(conn, ssl_context_.get());
      ssl_conn->message_callback(
          std::bind(&HttpServer::onMessage, this, std::placeholders::_1,
                    std::placeholders::_2, std::placeholders::_3));
      ssl_connections_[conn] = std::move(ssl_conn);
      ssl_connections_[conn]->start_handshake();
    }
    conn->setContext(HttpContext());  // 设置上下文
  } else {
    if (useSSL_) {
      ssl_connections_.erase(conn);
    }
  }
}

void HttpServer::onMessage(const muduo::net::TcpConnectionPtr& conn,
                           muduo::net::Buffer* buf,
                           muduo::Timestamp receive_time) {
  try {
    if (useSSL_) {
      LOG_INFO << "onMessage useSSL_ is true";

      // 查找对应的ssl连接
      auto it = ssl_connections_.find(conn);
      if (it != ssl_connections_.end()) {
        // ssl连接处理数据
        it->second->on_read(conn, buf, receive_time);

        // 如果ssl握手还没有完成，直接返回
        if (!it->second->is_hand_shake_done()) {
          return;
        }

        // 从ssl连接的解密缓冲区获取数据
        muduo::net::Buffer* decrypted_buf = it->second->decrypted_buffer();
        if (decrypted_buf->readableBytes() == 0) {
          return;
        }

        buf = decrypted_buf;
      }
    }

    HttpContext* context =
        boost::any_cast<HttpContext>(conn->getMutableContext());
    if (!context->parse_request(buf, receive_time)) {
      // 解析HTTP报文出错
      conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
      conn->shutdown();
    }

    if (context->parse_request_end()) {
      onRequest(conn, context->request());
      context->reset();
    }
  } catch (const std::exception& e) {
    LOG_ERROR << "onMessage error: " << e.what();
    conn->send("HTTP/1.1 500 Internal Server Error\r\n\r\n");
    conn->shutdown();
  }
}

void HttpServer::onRequest(const muduo::net::TcpConnectionPtr& conn,
                           const HttpRequest& request) {
  const std::string& connection = request.headers("Connection");
  bool close = (connection == "close" || (request.version() == "HTTP/1.0" &&
                                          connection != "Keep-Alive"));
  HttpResponse response(close);

  http_callback_(request, response);  // 执行回调函数处理

  muduo::net::Buffer buf;
  buf.append(response.str());
  conn->send(&buf);  // 发送响应

  if (response.close_connection()) {
    conn->shutdown();  // 关闭连接
  }
}

void HttpServer::handle_request(const HttpRequest& request,
                                HttpResponse& response) {
  try {
    HttpRequest mutable_request(request);  // 复制请求对象
    middleware_chain_.before(mutable_request);
    if (!router_.route(mutable_request, response)) {
      LOG_INFO << "No route found for " << request.method() << " "
               << request.path();
      response.status_code(HttpResponse::NOT_FOUND);
      response.text("Not Found");
      response.close_connection(true);
    }

    middleware_chain_.after(response);
  } catch (const std::exception& e) {
    response.status_code(HttpResponse::INTERNAL_SERVER_ERROR);
    response.text("Internal Server Error");
    response.close_connection(true);
    response.body(e.what());
  } catch (const HttpResponse& res) {
    response = res;
  }
}
