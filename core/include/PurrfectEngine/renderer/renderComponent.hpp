#ifndef PURRENGINE_RENDERER_RENDERCOMPONENT_HPP_
#define PURRENGINE_RENDERER_RENDERCOMPONENT_HPP_

#include "PurrfectEngine/object.hpp"

namespace PurrfectEngine {

  class purrMesh;
  // not implemented yet: class purrMaterial;

  class RenderComponent : public purrComponent {
  public:
    RenderComponent(purrMesh* mesh);
    // not implemented yet: RenderComponent(purrMesh* mesh, purrMaterial* material);
    virtual ~RenderComponent() override;

    virtual const char* getName() override { return "renderComponent"; }

    purrMesh* getMesh() const { return mMesh; }
    // not implemented yet: purrMaterial* getMaterial() const { return mMaterial; }

  private:
    purrMesh* mMesh;
    // not implemented yet: purrMaterial* mMaterial;
  };

} // namespace PurrfectEngine

#endif // PURRENGINE_RENDERCOMPONENT_HPP_
