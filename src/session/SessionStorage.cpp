#include "SessionStorage.h"

void MemorySessionStorage::save(std::shared_ptr<Session> session) {
  sessions_[session->id()] = session;  // 创建会话副本并保存到内存中
}

std::shared_ptr<Session> MemorySessionStorage::load(
    const std::string& session_id) {
  auto it = sessions_.find(session_id);
  if (it != sessions_.end()) {
    if (!it->second->is_expired()) {
      return it->second;  // 返回会话副本
    } else {
      sessions_.erase(it);  // 删除过期会话
    }
  }

  return nullptr;  // 返回空指针表示会话不存在或已过期
}

void MemorySessionStorage::remove(const std::string& session_id) {
  sessions_.erase(session_id);  // 移除会话
}
