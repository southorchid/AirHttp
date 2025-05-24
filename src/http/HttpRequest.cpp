#include "HttpRequest.h"

HttpRequest::HttpRequest()
    : method_(GET),
      version_("HTTP/1.1"),
      path_("/"),
      content_length_(0),
      receive_time_(muduo::Timestamp::now()) {}

bool HttpRequest::method(const std::string &method) {
  auto it = method_map_.find(method);
  if (it != method_map_.end()) {
    method_ = it->second;
    return true;
  }
  return false;
}

HttpRequest::METHOD HttpRequest::method() const { return method_; }

void HttpRequest::version(const std::string &version) { version_ = version; }

std::string HttpRequest::version() const { return version_; }

void HttpRequest::path(const std::string &path) { path_ = path; };

std::string HttpRequest::path() const { return path_; }

void HttpRequest::path_parameters(const std::string &key,
                                  const std::string &value) {
  path_parameters_[key] = value;
}

std::string HttpRequest::path_parameters(const std::string &key) const {
  auto it = path_parameters_.find(key);
  if (it != path_parameters_.end()) {
    return it->second;
  }
  return "";
}

void HttpRequest::query_parameters(const std::string &key,
                                   const std::string &value) {
  query_parameters_[key] = value;
}

std::string HttpRequest::query_parameters(const std::string &key) const {
  auto it = query_parameters_.find(key);
  if (it != query_parameters_.end()) {
    return it->second;
  }
  return "";
}

void HttpRequest::header(const std::string &key, const std::string &value) {
  headers_[key] = value;
}

std::string HttpRequest::header(const std::string &key) const {
  auto it = headers_.find(key);
  if (it != headers_.end()) {
    return it->second;
  }
  return "";
}

void HttpRequest::cookie(const std::string &cookie) {
  headers_["Cookie"] = cookie;
}

std::string HttpRequest::cookie() const {
  auto it = headers_.find("Cookie");
  if (it != headers_.end()) {
    return it->second;
  }
  return std::string();
}

void HttpRequest::body(const std::string &content) { content_ = content; }

std::string HttpRequest::body() const { return content_; }

void HttpRequest::content_length(uint16_t content_length) {
  content_length_ = content_length;
}

uint16_t HttpRequest::content_length() const { return content_length_; }

void HttpRequest::receive_time(muduo::Timestamp receive_time) {
  receive_time_ = receive_time;
}

muduo::Timestamp HttpRequest::receive_time() const { return receive_time_; }

void HttpRequest::swap(HttpRequest &that) {
  std::swap(method_, that.method_);
  std::swap(version_, that.version_);
  std::swap(path_, that.path_);
  std::swap(path_parameters_, that.path_parameters_);
  std::swap(query_parameters_, that.query_parameters_);
  std::swap(headers_, that.headers_);
  std::swap(content_, that.content_);
  std::swap(content_length_, that.content_length_);
  std::swap(receive_time_, that.receive_time_);
}

const std::unordered_map<std::string, HttpRequest::METHOD>
    HttpRequest::method_map_ = {{"GET", GET},       {"POST", POST},
                                {"HEAD", HEAD},     {"PUT", PUT},
                                {"DELETE", DELETE}, {"OPTIONS", OPTIONS}};

const std::unordered_map<HttpRequest::METHOD, std::string>
    HttpRequest::method_string_map_ = {
        {GET, "GET"}, {POST, "POST"},     {HEAD, "HEAD"},
        {PUT, "PUT"}, {DELETE, "DELETE"}, {OPTIONS, "OPTIONS"}};