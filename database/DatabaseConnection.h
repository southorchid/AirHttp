#pragma once

#include <cppconn/connection.h>
#include <cppconn/prepared_statement.h>
#include <cppconn/resultset.h>
#include <muduo/base/Logging.h>
#include <mysql/mysql.h>
#include <mysql_driver.h>

#include <memory>
#include <mutex>
#include <string>

#include "DatabaseException.h"

class DatabaseConnection {
 public:
  DatabaseConnection(const std::string &host, const std::string &user,
                     const std::string &password, const std::string &database);
  ~DatabaseConnection();

  DatabaseConnection(const DatabaseConnection &) = delete;
  DatabaseConnection &operator=(const DatabaseConnection &) = delete;

  // 执行select语句
  template <class... Args>
  sql::ResultSet *query(const std::string &sql, Args &&...args);

  // 执行update/insert/delete语句
  template <class... Args>
  int update(const std::string &sql, Args... args);

  // 测试数据库连接是否可用
  bool ping();

  // 数据库连接是否有效
  bool is_valid() const;

  // 重新连接数据库
  void reconnect();

  // 释放数据库资源
  void cleanup();

  // 递归终止函数
  void bind_params(sql::PreparedStatement *stmt, int);

  // 递归绑定参数
  template <class T, class... Args>
  void bind_params(sql::PreparedStatement *stmt, int index, T &&value,
                   Args &&...args);

 private:
  std::shared_ptr<sql::Connection> conn_;
  std::mutex mutex_;
  std::string host_;
  std::string user_;
  std::string password_;
  std::string database_;
};

template <class... Args>
inline sql::ResultSet *DatabaseConnection::query(const std::string &sql,
                                                 Args &&...args) {
  std::lock_guard<std::mutex> lock(mutex_);
  try {
    std::unique_ptr<sql::PreparedStatement> stmt(conn_->prepareStatement(sql));
    bind_params(stmt.get(), 1, std::forward<Args>(args)...);
    return stmt->executeQuery();
  } catch (const sql::SQLException &e) {
    LOG_ERROR << "Query failed: " << e.what() << ", SQL:" << sql;
    throw DatabaseException(e.what());
  }
}

template <class... Args>
inline int DatabaseConnection::update(const std::string &sql, Args... args) {
  std::lock_guard<std::mutex> lock(mutex_);
  try {
    std::unique_ptr<sql::PreparedStatement> stmt(conn_->prepareStatement(sql));
    bind_params(stmt.get(), 1, args...);
    return stmt->executeUpdate();
  } catch (const sql::SQLException &e) {
    LOG_ERROR << "Update failed: " << e.what() << ", SQL:" << sql;
    throw DatabaseException(e.what());
  }
}

template <class T, class... Args>
inline void DatabaseConnection::bind_params(sql::PreparedStatement *stmt,
                                            int index, T &&value,
                                            Args &&...args) {
  try {
    // index 参数索引，从 1 开始（注意不是 0）
    stmt->setString(index, value);
    bind_params(stmt, index + 1, std::forward<Args>(args)...);
  } catch (const sql::SQLException &e) {
    LOG_ERROR << "Bind params failed: " << e.what() << ", value:" << value;
    throw DatabaseException(e.what());
  }
}
