#include "PurrfectEngine/PurrfectEngine.hpp"

#include <fstream>

namespace PurrfectEngine::Utils {

  std::vector<char> ReadFile(const char *filename) {
    std::ifstream file(filename, std::ios::ate | std::ios::binary);
    if (!file.is_open()) return {};
    size_t fileSize = (size_t) file.tellg();
    std::vector<char> buffer(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();

    return buffer;
  }

}