#pragma once

#include <muduo/net/TcpServer.h>

#include <sstream>

#include "HttpRequest.h"

class HttpContext {
 public:
  enum HTTP_REQUEST_PARSE_STATE {
    EXPECT_REQUEST_LINE,     // 解析请求行
    EXPECT_REQUEST_HEADERS,  // 解析请求头
    EXPECT_REQUEST_BODY,     // 解析请求体
    PARSE_REQUEST_END,       // 解析完成
  };

  HttpContext();

  bool parse_request(muduo::net::Buffer *buf, muduo::Timestamp receive_time);

  bool parse_request_end() const;

  void reset();

  const HttpRequest &request() const;

 private:
  bool parse_request_line(const std::string &request_line);
  bool parse_query_parameters(const std::string &query_string);
  std::string trim(const std::string &str) const;

 private:
  HTTP_REQUEST_PARSE_STATE state_;
  HttpRequest request_;
};