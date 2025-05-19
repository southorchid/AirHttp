#include <iostream>

#include "DatabaseConnectionPool.h"

int main() {
  DatabaseConnectionPool::getInstance().init("0.0.0.0", "root", "617020@Dai",
                                             "Gomoku", 10);
  auto conn = DatabaseConnectionPool::getInstance().acquire();
  conn->update("insert into users(username, password) values(?,?)", "south",
               "123456");
  return 0;
}