#include "WebServer.h"

int main() {
  LOG_INFO << "pid = " << getpid();

  std::string serverName = "WebServer";
  int port = 8080;

  muduo::Logger::setLogLevel(muduo::Logger::DEBUG);

  WebServer server(port, serverName);

  server.setThreadNum(4);

  server.start();
  return 0;
}