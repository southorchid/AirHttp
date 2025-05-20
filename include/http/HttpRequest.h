#pragma once
#include <muduo/base/Timestamp.h>  // Include the header for muduo::Timestamp

#include <sstream>
#include <string>
#include <unordered_map>

class HttpRequest {
 public:
  enum METHOD { GET, POST, HEAD, PUT, DELETE, OPTIONS, INVALID };

  HttpRequest();

  // 设置/获取请求方法
  bool method(const std::string &method);
  METHOD method() const;

  // 设置/获取http版本
  void version(const std::string &version);
  std::string version() const;

  // 设置/获取请求路径
  void path(const std::string &path);
  std::string path() const;

  // 设置/获取路径参数
  void path_parameters(const std::string &key, const std::string &value);
  std::string path_parameters(const std::string &key) const;

  // 设置/获取查询参数
  void query_parameters(const std::string &key, const std::string &value);
  std::string query_parameters(const std::string &key) const;

  // 设置/获取请求头
  void headers(const std::string &key, const std::string &value);
  std::string headers(const std::string &key) const;

  // 设置/获取cookie
  void cookie(const std::string &cookies);
  std::string cookie() const;

  // 设置/获取请求体
  void body(const std::string &content);
  std::string body() const;

  // 设置/获取请求体长度
  void content_length(uint16_t content_length);
  uint16_t content_length() const;

  // 设置/获取接收时间
  void receive_time(muduo::Timestamp receive_time);
  muduo::Timestamp receive_time() const;

  void swap(HttpRequest &that);

 private:
  METHOD method_;                                                  // 请求方法
  std::string version_;                                            // http版本
  std::string path_;                                               // 请求路径
  std::unordered_map<std::string, std::string> path_parameters_;   // 路径参数
  std::unordered_map<std::string, std::string> query_parameters_;  // 查询参数
  std::unordered_map<std::string, std::string> headers_;           // 请求头
  std::string content_;                                            // 请求体
  uint16_t content_length_;                                        // 请求体长度
  muduo::Timestamp receive_time_;                                  // 接收时间
  static const std::unordered_map<std::string, METHOD>
      method_map_;  // 请求方法映射
  static const std::unordered_map<METHOD, std::string> method_string_map_;
};