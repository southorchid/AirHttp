#pragma once
#include <memory>
#include <random>

#include "HttpRequest.h"
#include "HttpResponse.h"
#include "Session.h"
#include "SessionStorage.h"

class Session;
class SessionStorage;
class SessionManager {
 public:
  explicit SessionManager(std::unique_ptr<SessionStorage> storage);

  // 获取会话
  std::shared_ptr<Session> get_session(const HttpRequest& request,
                                       HttpResponse& response);

  // 销毁会话
  void destroy_session(const std::string& id);

  // 清理过期会话
  void clean_expired_sessions();

  // 更新会话
  void update_session(std::shared_ptr<Session> session);

 private:
  // 随机生产会话ID
  std::string generate_session_id();

  // 从cookies获取会话ID
  std::string get_session_id_from_cookie(const HttpRequest& request);

  // 设置会话到cookies
  void set_session_cookie(HttpResponse& response,
                          const std::string& session_id);

 private:
  std::unique_ptr<SessionStorage> storage_;  // 会话存储
  std::mt19937 random_generator_;            // 随机数生成器
};