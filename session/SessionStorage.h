#pragma once

#include <memory>
#include <string>
#include <unordered_map>

#include "Session.h"

class SessionStorage {
 public:
  virtual void save(std::shared_ptr<Session> session) = 0;

  virtual std::shared_ptr<Session> load(const std::string &session_id) = 0;

  virtual void remove(const std::string &session_id) = 0;

 private:
};

class MemorySessionStorage : public SessionStorage {
 public:
  void save(std::shared_ptr<Session> session) override;

  std::shared_ptr<Session> load(const std::string &session_id) override;

  void remove(const std::string &session_id) override;

 private:
  std::unordered_map<std::string, std::shared_ptr<Session>> sessions_;
};