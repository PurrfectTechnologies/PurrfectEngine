#include "PurrfectEngine/PurrfectEngine.hpp"

namespace PurrfectEngine {

  bool vec_find_str(std::vector<const char *> vec, const char *str, size_t *pos) {
    for (std::vector<const char *>::iterator it = vec.begin(); it != vec.end(); ++it) {
      if (strcmp(*it, str) == 0) {
        *pos = it - vec.begin();
        return true;
      }
    }
    return false;
  }

}