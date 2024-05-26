#ifndef   PURRENGINE_ASSETS_HPP_
#define   PURRENGINE_ASSETS_HPP_

namespace PurrfectEngine {

  enum class purrAssetType {
    Mesh,
    Scene,
    Texture,
  };

  struct purrAssetFile {
    purrAssetType type;
    uint32_t puid;
    uint32_t version;

    uint32_t json_len;
    char *json;

    uint32_t blob_len;
    char *blob;

    bool load(const char *filename);
    bool save(const char *filename);
  };

  struct purrAssetHandle {
    purrAssetType type;
    PUID puid;
    uint32_t version;
  };

  class purrAsset {
  public:
    virtual ~purrAsset() = 0;

    bool load(const char *filename);
    bool save(const char *filename);
  protected:
    purrAssetHandle mHandle{};

    virtual bool _load(purrAssetFile file) = 0;
    virtual bool _save(purrAssetFile *file) = 0;
  };

  class TextureAsset {

  };

}

#endif // PURRENGINE_ASSETS_HPP_