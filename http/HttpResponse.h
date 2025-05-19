#pragma once

#include <sstream>
#include <string>
#include <unordered_map>

class HttpResponse {
 public:
  enum STATUS_CODE {
    OK = 200,
    NO_CONTENT = 204,
    MOVED_PERMANENTLY = 301,
    NOT_MODIFIED = 304,
    BAD_REQUEST = 400,
    UNAUTHORIZED = 401,
    FORBIDDEN = 403,
    NOT_FOUND = 404,
    INTERNAL_SERVER_ERROR = 500,
    NOT_IMPLEMENTED = 501,
    BAD_GATEWAY = 502,
    SERVICE_UNAVAILABLE = 503,
    GATEWAY_TIMEOUT = 504,
  };

  HttpResponse(bool close = true);

  // 设置/获取http版本
  void version(const std::string &version);
  std::string version() const;

  // 设置/获取状态码
  void status_code(STATUS_CODE status_code);
  STATUS_CODE status_code() const;

  // 设置/获取状态消息
  void text(const std::string &text);
  std::string text() const;

  // 设置/获取响应头
  void header(const std::string &key, const std::string &value);
  std::string header(const std::string &key) const;

  // 设置/获取响应体
  void body(const std::string &body);
  std::string body() const;

  // 设置/获取是否关闭连接
  void close_connection(bool close);
  bool close_connection() const;

  // 设置/获取是否是文件
  void is_file(bool is_file);
  bool is_file() const;

  void response_line(const std::string &version, STATUS_CODE status_code,
                     const std::string &text);

  // 序列化响应
  std::string str() const;

 private:
  std::string version_;                                   // http版本
  STATUS_CODE status_code_;                               // 状态码
  std::string text_;                                      // 状态消息
  std::unordered_map<std::string, std::string> headers_;  // 响应头
  std::string body_;                                      // 响应体
  bool close_connection_;                                 // 是否关闭连接
  bool is_file_;                                          // 是否是文件
  static const std::unordered_map<STATUS_CODE, std::string>
      status_code_map_;  // 状态码映射
};