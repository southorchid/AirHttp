#include "SessionManager.h"

SessionManager::SessionManager(std::unique_ptr<SessionStorage> storage)
    : storage_(std::move(storage)), random_generator_(std::random_device{}()) {}

std::shared_ptr<Session> SessionManager::get_session(const HttpRequest& request,
                                                     HttpResponse& response) {
  /// 从请求中获取会话ID
  std::string session_id = get_session_id_from_cookie(request);

  std::shared_ptr<Session> session;

  if (!session_id.empty()) {
    // 尝试从存储中加载会话
    session = storage_->load(session_id);
  }

  if (!session || session->is_expired()) {
    // 如果会话不存在或已过期，则创建一个新的会话
    session_id = generate_session_id();
    session = std::make_shared<Session>(session_id, this);
    set_session_cookie(response, session_id);
  } else {
    // 如果会话存在且未过期，则设置会话管理器
    session->manager(this);
  }

  session->refresh();       // 刷新会话过期时间
  storage_->save(session);  // 保存会话到存储
  return session;
}

void SessionManager::destroy_session(const std::string& id) {
  storage_->remove(id);  // 从存储中删除会话
}

void SessionManager::clean_expired_sessions() {}

void SessionManager::update_session(std::shared_ptr<Session> session) {
  storage_->save(session);  // 更新会话到存储
}

std::string SessionManager::generate_session_id() {
  std::ostringstream ss;
  std::uniform_int_distribution<int> dist(0, 15);

  for (int i = 0; i < 32; ++i) {
    ss << std::hex << dist(random_generator_);
  }
  return ss.str();  // 返回生成的会话ID
}

std::string SessionManager::get_session_id_from_cookie(
    const HttpRequest& request) {
  std::string session_id;
  auto cookies = request.cookie();
  if (!cookies.empty()) {
    auto pos = cookies.find("session_id=");
    if (pos != std::string::npos) {
      pos += 11;  // 跳过"session_id="
      auto end = cookies.find(';', pos);
      if (end != std::string::npos) {
        session_id = cookies.substr(pos, end - pos);
      } else {
        session_id = cookies.substr(pos);
      }
    }
  }
  return session_id;  // 返回会话ID
}

void SessionManager::set_session_cookie(HttpResponse& response,
                                        const std::string& session_id) {
  std::string cookie = "session_id=" + session_id + "; Path=/; HttpOnly";
  response.header("Set-Cookie", cookie);  // 设置cookie到响应头
}
