#include "DatabaseConnection.h"

DatabaseConnection::DatabaseConnection(const std::string& host,
                                       const std::string& user,
                                       const std::string& password,
                                       const std::string& database)
    : host_(host), user_(user), password_(password), database_(database) {
  try {
    sql::mysql::MySQL_Driver* driver = sql::mysql::get_driver_instance();
    conn_.reset(driver->connect(host_, user_, password_));
    if (conn_) {
      // 指定要使用的数据库
      conn_->setSchema(database_);
      // 启用自动重连
      conn_->setClientOption("OPT_RECONNECT", "true");
      // 设置连接超时时间为 10 秒
      conn_->setClientOption("OPT_CONNECT_TIMEOUT", "10");
      // 禁用多语句执行（防止 SQL 注入风险）
      conn_->setClientOption("multi_statement", "false");

      // 设置字符集
      std::unique_ptr<sql::Statement> stmt(conn_->createStatement());
      stmt->execute("SET NAMES utf8mb4");
      LOG_INFO << "Database connection established";
    }
  } catch (const sql::SQLException& e) {
    LOG_ERROR << "Failed to create database connection: " << e.what();
    throw DatabaseException(e.what());
  }
}

DatabaseConnection::~DatabaseConnection() {
  cleanup();
  LOG_INFO << "Database connection closed";
}

bool DatabaseConnection::ping() {
  try {
    std::unique_ptr<sql::Statement> stmt(conn_->createStatement());
    std::unique_ptr<sql::ResultSet> res(stmt->executeQuery("SELECT 1"));
    return true;
  } catch (const sql::SQLException& e) {
    LOG_ERROR << "Failed to ping database: " << e.what();
    return false;
  }
}

bool DatabaseConnection::is_valid() const {
  try {
    if (!conn_) {
      return false;
    }
    std::unique_ptr<sql::Statement> stmt(conn_->createStatement());
    stmt->execute("SELECT 1");
    return true;
  } catch (const sql::SQLException& e) {
    return false;
  }
}

void DatabaseConnection::reconnect() {
  try {
    if (conn_) {
      conn_->reconnect();  // 尝试重连
    } else {
      sql::mysql::MySQL_Driver* driver = sql::mysql::get_driver_instance();
      conn_.reset(driver->connect(host_, user_, password_));
      conn_->setSchema(database_);
    }
  } catch (const sql::SQLException& e) {
    LOG_ERROR << "Failed to reconnect database: " << e.what();
    throw DatabaseException(e.what());
  }
}

void DatabaseConnection::cleanup() {
  std::unique_lock<std::mutex> lock(mutex_);
  try {
    if (conn_) {
      if (!conn_->getAutoCommit()) {
        conn_->rollback();           // 回滚未提交的事务
        conn_->setAutoCommit(true);  // 启用自动提交
      }

      // 清理所有未处理的结果集
      std::unique_ptr<sql::Statement> stmt(conn_->createStatement());
      while (stmt->getMoreResults()) {
        auto result = stmt->getResultSet();
        while (result && result->next());
      }
    }
  } catch (const sql::SQLException& e) {
    LOG_ERROR << "Failed to cleanup database connection: " << e.what();
    try {
      conn_->reconnect();
    } catch (const sql::SQLException& e) {
      // 忽略重连异常，释放资源不抛出异常
    }
  }
}

void DatabaseConnection::bind_params(sql::PreparedStatement* stmt, int) {}
