#pragma once

#include <string>

#include "DatabaseConnectionPool.h"

class DatabaseService {
 public:
  enum class LoginResult {
    SUCCESS,
    USER_NOT_REGISTERED,
    PASSWORD_INCORRECT,
    SERVER_ERROR
  };

  static LoginResult login(const std::string& username,
                           const std::string& password, int& userId);

 private:
};