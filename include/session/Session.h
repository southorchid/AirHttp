#pragma once

#include <chrono>
#include <memory>
#include <string>
#include <unordered_map>

#include "SessionManager.h"

class SessionManager;
class Session : public std::enable_shared_from_this<Session> {
 public:
  Session(const std::string &id, SessionManager *manager, int max_age = 3600);

  const std::string &id() const;

  // 会话是否过期
  bool is_expired() const;

  // 刷新会话过期时间
  void refresh();

  // 设置/获取管理器
  void manager(SessionManager *manager);
  SessionManager *manager() const;

  // 设置/获取会话数据
  void value(const std::string &key, const std::string &value);
  std::string value(const std::string &key) const;

  // 清除指定的会话数据
  void remove(const std::string &key);

  // 清除所有会话数据
  void clear();

 private:
  std::string id_;
  std::unordered_map<std::string, std::string> data_;
  std::chrono::system_clock::time_point expire_time_;
  int max_age_;
  SessionManager *manager_;
};