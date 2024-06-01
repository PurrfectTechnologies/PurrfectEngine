#ifndef   PURRENGINE_RENDERER_MATERIAL_HPP_
#define   PURRENGINE_RENDERER_MATERIAL_HPP_

namespace PurrfectEngine {

  class purrMaterial {
  public:
    purrMaterial();
    ~purrMaterial();

    // void setAlbedo(purrTexture *t)    { mAlbedo = t; }
    void setNormal(purrTexture *t)    { mNormal = t; }
    void setRoughness(purrTexture *t) { mRoughness = t; }
    void setMetalic(purrTexture *t)   { mMetalic = t; }
  private:
    // purrTexture *mAlbedo;
    purrTexture *mNormal;
    purrTexture *mRoughness;
    purrTexture *mMetalic;
  };

}

#endif // PURRENGINE_RENDERER_MATERIAL_HPP_