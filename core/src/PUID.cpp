#include "PurrfectEngine/PurrfectEngine.hpp"

#include <random>

namespace PurrfectEngine {

  static std::random_device sRandomDevice;
  static std::mt19937_64 sEngine(sRandomDevice());
  static std::uniform_int_distribution<uint32_t> sUniformDistribution;

  PUID::PUID():
    mId(sUniformDistribution(sEngine))
  {}

}