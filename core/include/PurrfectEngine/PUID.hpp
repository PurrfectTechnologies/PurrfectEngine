#ifndef   PURRENGINE_PUID_HPP_
#define   PURRENGINE_PUID_HPP_

namespace PurrfectEngine {

  struct PUID {
  public:
    PUID();
    constexpr PUID(uint32_t id) : mId(id) {}

    bool operator==(const PUID &other) const {
      return mId == other.mId;
    }

    bool operator!=(const PUID &other) const {
      return !(*this == other);
    }

    operator const uint32_t &() const {
      return mId;
    }

    uint32_t get() const { return mId; }
  private:
    uint32_t mId = 0;
  };

}

namespace std {
  template <>
  struct hash<PurrfectEngine::PUID> {
    size_t operator()(const PurrfectEngine::PUID& uuid) const {
      return static_cast<size_t>(uuid.get());
    }
  };
}

#endif // PURRENGINE_PUID_HPP_