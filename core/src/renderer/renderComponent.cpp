#include "PurrfectEngine/renderer/renderComponent.hpp"
#include "PurrfectEngine/renderer/mesh.hpp"
#include <cassert>

namespace PurrfectEngine {

  RenderComponent::RenderComponent(purrMesh* mesh)
    : mMesh(mesh) {
    assert(mesh && "Mesh must not be null");
  }

  RenderComponent::~RenderComponent() {
    delete mMesh;
  }

}