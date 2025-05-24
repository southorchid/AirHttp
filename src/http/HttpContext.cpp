#include "HttpContext.h"

HttpContext::HttpContext() : state_(EXPECT_REQUEST_LINE) {}

bool HttpContext::parse_request(muduo::net::Buffer* buf,
                                muduo::Timestamp receive_time) {
  bool ok = true;        // 请求格式是否正确
  bool has_more = true;  // 是否还有未解析的数据
  while (has_more) {
    if (state_ == EXPECT_REQUEST_LINE) {
      const char* crlf = buf->findCRLF();  // 查找 \r\n
      if (crlf) {
        ok = parse_request_line(std::string(buf->peek(), crlf));  // 解析请求行
        if (ok) {
          request_.receive_time(receive_time);
          buf->retrieveUntil(crlf + 2);  // +2是为了跳过\r\n
          state_ = EXPECT_REQUEST_HEADERS;
        } else {
          has_more = false;  // 请求行解析失败，终止
        }
      } else {
        has_more = false;  // 没找到 \r\n，说明数据还不完整，等待更多数据
      }
    } else if (state_ == EXPECT_REQUEST_HEADERS) {
      const char* crlf = buf->findCRLF();  // 查找请求头一行末尾的 \r\n
      if (crlf) {
        const char* colon = std::find(buf->peek(), crlf, ':');  // 查找冒号
        if (colon < crlf) {
          request_.header(trim(std::string(buf->peek(), colon)),
                          trim(std::string(colon + 1, crlf)));
        } else if (colon == crlf) {  // 解析到空行，表示请求头结束
          if (request_.method() == HttpRequest::POST ||
              request_.method() == HttpRequest::PUT) {
            // 只有在POST和PUT请求中才有Content-Length
            std::string content_length = request_.header("Content-Length");
            if (!content_length.empty()) {
              request_.content_length(std::stoi(content_length));
              if (request_.content_length() > 0) {
                state_ = EXPECT_REQUEST_BODY;
              } else {
                state_ = PARSE_REQUEST_END;
                has_more = false;
              }
            } else {
              // 没有Content-Length，说明请求语法错误
              ok = false;
              has_more = false;
            }
          } else {
            // GET/HEAD/DELETE请求没有请求体，直接结束解析
            state_ = PARSE_REQUEST_END;
            has_more = false;
          }
        } else {
          // 解析到错误的请求头格式，终止解析
          ok = false;
          has_more = false;
        }
        buf->retrieveUntil(crlf + 2);  // 进入下一行解析
      } else {
        has_more = false;
      }
    } else if (state_ == EXPECT_REQUEST_BODY) {
      if (buf->readableBytes() < request_.content_length()) {
        has_more = false;  // 请求体还不完整，等待更多数据
        return true;
      }
      request_.body(
          std::string(buf->peek(), buf->peek() + request_.content_length()));
      buf->retrieve(request_.content_length());
      state_ = PARSE_REQUEST_END;  // 请求解析完成
      has_more = false;            // 结束解析
    }
  }
  return ok;
}

bool HttpContext::parse_request_end() const {
  return state_ == PARSE_REQUEST_END;  // 判断请求是否解析完成
}

void HttpContext::reset() {
  state_ = EXPECT_REQUEST_LINE;  // 重置状态
  HttpRequest dummy_data;
  request_.swap(dummy_data);  // 清空请求数据
}

const HttpRequest& HttpContext::request() const {
  return request_;  // 返回请求对象
}

bool HttpContext::parse_request_line(const std::string& request_line) {
  std::istringstream line_stream(request_line);

  std::string method;
  line_stream >> method;
  if (method.empty()) {
    return false;  // 请求方法为空
  }

  if (!request_.method(method)) {
    return false;  // 请求方法不合法
  }

  std::string url;
  line_stream >> url;
  if (url.empty()) {
    return false;  // 请求路径为空
  }

  auto pos = url.find('?');
  if (pos == std::string::npos) {
    // 没有查询参数
    request_.path(url);  // 设置请求路径
  } else {
    // 有查询参数
    request_.path(url.substr(0, pos));  // 设置请求路径
    if (!parse_query_parameters(url.substr(pos + 1))) {
      return false;  // 查询参数解析失败
    }
  }

  std::string version;
  line_stream >> version;
  if (version != "HTTP/1.1" && version != "HTTP/1.0") {
    return false;  // HTTP版本不合法
  }
  request_.version(version);  // 设置HTTP版本
  return true;                // 请求行解析成功
}

bool HttpContext::parse_query_parameters(const std::string& query_string) {
  std::istringstream query_stream(query_string);
  std::string key_value_pair;
  while (std::getline(query_stream, key_value_pair, '&')) {
    if (key_value_pair.empty()) {
      continue;  // 跳过空的查询参数
    }
    auto pos = key_value_pair.find('=');
    if (pos == std::string::npos) {
      return false;  // 错误的查询参数格式
    }
    std::string key = key_value_pair.substr(0, pos);
    std::string value = key_value_pair.substr(pos + 1);
    request_.query_parameters(key, value);  // 设置查询参数
  }
  return true;  // 查询参数解析成功
}

std::string HttpContext::trim(const std::string& str) const {
  size_t start = str.find_first_not_of(' ');
  if (start == std::string::npos) {
    return "";  // 字符串全是空格
  }
  size_t end = str.find_last_not_of(' ');
  return str.substr(start, end - start + 1);  // 去除首尾空格
}
