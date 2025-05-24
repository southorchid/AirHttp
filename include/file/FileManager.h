#pragma once

#include <fstream>
#include <sstream>
#include <string>

class FileManager {
 public:
  static int readFile(const std::string& filePath, std::string& buffer);

 private:
};