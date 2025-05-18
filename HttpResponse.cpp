#include "HttpResponse.h"

HttpResponse::HttpResponse(bool close)
    : version_("HTTP/1.1"),
      status_code_(OK),
      text_("OK"),
      close_connection_(close),
      is_file_(false) {}

void HttpResponse::version(const std::string &version) { version_ = version; }

std::string HttpResponse::version() const { return version_; }

void HttpResponse::status_code(STATUS_CODE status_code) {
  status_code_ = status_code;
}

HttpResponse::STATUS_CODE HttpResponse::status_code() const {
  return status_code_;
}

void HttpResponse::text(const std::string &text) { text_ = text; }

std::string HttpResponse::text() const { return text_; }

void HttpResponse::headers(const std::string &key, const std::string &value) {
  headers_[key] = value;
}

std::string HttpResponse::headers(const std::string &key) const {
  auto it = headers_.find(key);
  if (it != headers_.end()) {
    return it->second;
  }
  return "";
}

void HttpResponse::body(const std::string &body) { body_ = body; }

std::string HttpResponse::body() const { return body_; }

void HttpResponse::close_connection(bool close) { close_connection_ = close; }

bool HttpResponse::close_connection() const { return close_connection_; }

void HttpResponse::is_file(bool is_file) { is_file_ = is_file; }

bool HttpResponse::is_file() const { return is_file_; }

void HttpResponse::response_line(const std::string &version,
                                 STATUS_CODE status_code,
                                 const std::string &text) {
  version_ = version;
  status_code_ = status_code;
  text_ = text;
}

std::string HttpResponse::str() const {
  std::ostringstream response_stream;
  response_stream << version_ << " " << status_code_ << " " << text_ << "\r\n";
  if (close_connection_) {
    response_stream << "Connection: close\r\n";
  } else {
    response_stream << "Connection: keep-alive\r\n";
  }
  for (const auto &header : headers_) {
    response_stream << header.first << ": " << header.second << "\r\n";
  }
  response_stream << "\r\n";
  response_stream << body_;
  return response_stream.str();
}
