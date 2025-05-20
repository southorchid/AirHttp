#include "DatabaseConnectionPool.h"

DatabaseConnectionPool& DatabaseConnectionPool::getInstance() {
  static DatabaseConnectionPool instance;
  return instance;
}

void DatabaseConnectionPool::init(const std::string& host,
                                  const std::string& user,
                                  const std::string& password,
                                  const std::string& database,
                                  size_t max_size) {
  std::lock_guard<std::mutex> lock(mutex_);
  if (initialized_) {
    return;
  }

  host_ = host;
  user_ = user;
  password_ = password;
  database_ = database;
  max_size_ = max_size;

  for (int i = 0; i < max_size_; i++) {
    connections_.push(create_connection());
  }
  initialized_ = true;
  LOG_INFO << "Database connection pool initialized.";
}

std::shared_ptr<DatabaseConnection> DatabaseConnectionPool::acquire() {
  std::shared_ptr<DatabaseConnection> connection;
  {
    std::unique_lock<std::mutex> lock(mutex_);
    while (connections_.empty()) {
      if (!initialized_) {
        throw DatabaseException("Database connection pool not initialized.");
      }
      cv_.wait(lock);
    }
    connection = connections_.front();
    connections_.pop();
  }

  try {
    if (!connection->ping()) {
      LOG_WARN << "Database connection is not available.";
      connection->reconnect();
    }
    return std::shared_ptr<DatabaseConnection>(
        connection.get(), [this, connection](DatabaseConnection*) {
          std::lock_guard<std::mutex> lock(mutex_);
          connections_.push(connection);
          cv_.notify_one();
        });
  } catch (const std::exception& e) {
    LOG_ERROR << "Failed to acquire database connection: " << e.what();
    {
      std::lock_guard<std::mutex> lock(mutex_);
      connections_.push(connection);
      cv_.notify_one();
    }
    throw std::runtime_error("Failed to acquire database connection.");
  }
}

DatabaseConnectionPool::DatabaseConnectionPool() : initialized_(false) {
  check_thread_ = std::thread(&DatabaseConnectionPool::check_connections, this);
}

DatabaseConnectionPool::~DatabaseConnectionPool() {
  std::lock_guard<std::mutex> lock(mutex_);
  while (!connections_.empty()) {
    connections_.pop();
  }
  if (check_thread_.joinable()) {
    check_thread_.join();
  }
  LOG_INFO << "Database connection pool destroyed.";
}

std::shared_ptr<DatabaseConnection>
DatabaseConnectionPool::create_connection() {
  return std::make_shared<DatabaseConnection>(host_, user_, password_,
                                              database_);
}

void DatabaseConnectionPool::check_connections() {
  while (true) {
    try {
      std::vector<std::shared_ptr<DatabaseConnection>> connections;
      {
        std::lock_guard<std::mutex> lock(mutex_);
        if (connections_.empty()) {
          std::this_thread::sleep_for(std::chrono::seconds(1));
          continue;
        }

        auto temp = connections_;
        while (!temp.empty()) {
          connections.push_back(temp.front());
          temp.pop();
        }
      }

      // 在锁外检查连接是否可用
      for (auto& connection : connections) {
        if (!connection->ping()) {
          connection->reconnect();
        }
      }

      // 每隔60秒检查一次连接是否可用
      std::this_thread::sleep_for(std::chrono::seconds(60));
    } catch (const std::exception& e) {
      LOG_ERROR << "Failed to check database connections: " << e.what();
      std::this_thread::sleep_for(std::chrono::seconds(5));
    }
  }
}
