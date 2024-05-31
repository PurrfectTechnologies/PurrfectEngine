#include "PurrfectEngine/PurrfectEngine.hpp"

#include <nlohmann/json.hpp>

namespace PurrfectEngine {

  bool purrAssetFile::load(const char *filename) {
    FILE *fd = fopen(filename, "rb");
    if (!fd) {
      return false;
    }

    fseek(fd, 0, SEEK_SET);
    fread((char*)&type,    sizeof(type), 1, fd);
    fread((char*)&puid,    sizeof(puid), 1, fd);
    fread((char*)&version, sizeof(version), 1, fd);

    fread((char*)&json_len, sizeof(json_len), 1, fd);
    fread((char*)&blob_len, sizeof(blob_len), 1, fd);

    json = (char*)malloc(json_len);
    fread(json, json_len, 1, fd);

    blob = (char*)malloc(blob_len);
    fread(blob, blob_len, 1, fd);
    fclose(fd);

    return true;
  }

  bool purrAssetFile::save(const char *filename) {
    FILE *fd = fopen(filename, "wb");
    if (!fd) {
      return false;
    }

    fwrite((char*)&type, sizeof(type), 1, fd);
    fwrite((char*)&puid, sizeof(puid), 1, fd);
    fwrite((char*)&version, sizeof(version), 1, fd);

    fwrite((char*)&json_len, sizeof(json), 1, fd);

    fwrite((char*)&blob_len, sizeof(blob), 1, fd);

    fwrite(json, json_len, 1, fd);
    fwrite(blob, blob_len, 1, fd);

    fclose(fd);

    return true;
  }

  bool purrAsset::load(const char *filename) {
    purrAssetFile file = {};
    if (!file.load(filename)) return false;
    mHandle.type = file.type;
    mHandle.puid = file.puid;
    mHandle.version = file.version;
    return _load(file);
  }

  bool purrAsset::save(const char *filename) {
    purrAssetFile file = {};
    file.type = mHandle.type;
    file.puid = mHandle.puid();
    file.version = mHandle.version;
    if (!_save(&file)) return false;
    return file.save(filename);
  }

  TextureAsset::TextureAsset(int width, int height, VkFormat format)
    : texture(width, height, format) { }

  TextureAsset::~TextureAsset() { texture.cleanup(); }

  bool TextureAsset::load(const char *filename) {
      purrAssetFile file;
      if (!file.load(filename)) {
          return false;
      }
      return _load(file);
  }

  bool TextureAsset::save(const char *filename) {
      purrAssetFile file;
      if (!_save(&file)) {
          return false;
      }
      return file.save(filename);
  }

  bool TextureAsset::_load(purrAssetFile file) {
      texture.initialize(purrSampler::getDefault());
      texture.setPixels(reinterpret_cast<uint8_t*>(file.blob), file.blob_len);
      return true;
  }

  bool TextureAsset::_save(purrAssetFile *file) {
      std::vector<uint8_t> pixels;
      size_t pixelSize = 0;
      
      texture.getPixels(pixels, pixelSize);

      file->blob_len = static_cast<uint32_t>(pixels.size());
      file->blob = (char*)malloc(file->blob_len);
      memcpy(file->blob, pixels.data(), file->blob_len);

      return true;
  }

}