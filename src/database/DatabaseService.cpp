#include "DatabaseService.h"

DatabaseService::LoginResult DatabaseService::login(const std::string& username,
                                                    const std::string& password,
                                                    int& userId) {
  auto conn = DatabaseConnectionPool::getInstance().acquire();
  auto res = conn->query("SELECT id FROM users WHERE username = ?", username);
  if (res->rowsCount() == 0) {
    userId = -1;
    return LoginResult::USER_NOT_REGISTERED;
  }
  res = conn->query("SELECT id FROM users WHERE username = ? AND password = ?",
                    username, password);
  if (res->next()) {
    userId = res->getInt("id");
    return LoginResult::SUCCESS;
  }

  userId = -1;
  return LoginResult::PASSWORD_INCORRECT;
}