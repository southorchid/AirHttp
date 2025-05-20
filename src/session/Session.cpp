#include "Session.h"

#include "SessionStorage.h"

Session::Session(const std::string& id, SessionManager* manager, int max_age)
    : id_(id), manager_(manager), max_age_(max_age) {
  refresh();
}

const std::string& Session::id() const { return id_; }

bool Session::is_expired() const {
  return std::chrono::system_clock::now() > expire_time_;
}

void Session::refresh() {
  expire_time_ =
      std::chrono::system_clock::now() + std::chrono::seconds(max_age_);
}

void Session::manager(SessionManager* manager) { manager_ = manager; }

SessionManager* Session::manager() const { return manager_; }

void Session::value(const std::string& key, const std::string& value) {
  data_[key] = value;
  if (manager_) {
    manager_->update_session(shared_from_this());
  }
}

std::string Session::value(const std::string& key) const {
  auto it = data_.find(key);
  if (it != data_.end()) {
    return it->second;
  }
  return std::string();
}

void Session::remove(const std::string& key) { data_.erase(key); }

void Session::clear() { data_.clear(); }
