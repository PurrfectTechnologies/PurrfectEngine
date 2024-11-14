#ifndef   PURRENGINE_EXTENSIONS_HPP_
#define   PURRENGINE_EXTENSIONS_HPP_

namespace PurrfectEngine {

  template <typename extType>
  class purrExtendable {
  public:
    purrExtendable(std::vector<extType*> extensions):
      mExtensions(extensions)
    {}

    void addExtension(extType *extension) { mExtensions.push_back(extension); }
  protected:
    bool extsInitialize() {
      bool result = true;
      for (extType *ext: mExtensions) if (!ext->initialize()) result = false;
      return result;
    }

    bool extsPreUpdate() {
      bool result = true;
      for (extType *ext: mExtensions) if (!ext->preUpdate()) result = false;
      return result;
    }

    bool extsUpdate() {
      bool result = true;
      for (extType *ext: mExtensions) if (!ext->update()) result = false;
      return result;
    }

    void extsCleanup() {
      for (extType *ext: mExtensions) ext->cleanup();
    }

    std::vector<extType*> &getExtensions() { return mExtensions; }
  private:
    std::vector<extType*> mExtensions{};
  };

}

#endif // PURRENGINE_EXTENSIONS_HPP_