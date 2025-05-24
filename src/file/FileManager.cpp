#include "FileManager.h"

int FileManager::readFile(const std::string& filePath, std::string& buffer) {
  std::ifstream file(filePath, std::ios::in);
  if (file.fail()) {
    return -1;
  }
  std::ostringstream oss;
  oss << file.rdbuf();
  buffer = oss.str();
  file.close();
  return oss.str().size();
}