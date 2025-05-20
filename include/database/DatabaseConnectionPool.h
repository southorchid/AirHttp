#pragma once

#include <muduo/base/Logging.h>

#include <condition_variable>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>

#include "DatabaseConnection.h"

class DatabaseConnectionPool {
 public:
  static DatabaseConnectionPool &getInstance();

  // 初始化连接池
  void init(const std::string &host, const std::string &user,
            const std::string &password, const std::string &database,
            size_t max_size);

  // 获取连接
  std::shared_ptr<DatabaseConnection> acquire();

 private:
  DatabaseConnectionPool();
  ~DatabaseConnectionPool();
  DatabaseConnectionPool(const DatabaseConnectionPool &) = delete;
  DatabaseConnectionPool &operator=(const DatabaseConnectionPool &) = delete;

  // 创建一个新的连接
  std::shared_ptr<DatabaseConnection> create_connection();

  // 检查连接是否可用
  void check_connections();

 private:
  std::string host_;
  std::string user_;
  std::string password_;
  std::string database_;
  size_t max_size_;
  std::queue<std::shared_ptr<DatabaseConnection>> connections_;
  std::mutex mutex_;
  std::condition_variable cv_;
  bool initialized_;
  std::thread check_thread_;  // 监控连接是否可用线程
};