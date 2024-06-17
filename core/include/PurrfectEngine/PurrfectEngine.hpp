#ifndef   PURRENGINE_PURRFECTENGINE_HPP_
#define   PURRENGINE_PURRFECTENGINE_HPP_

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#undef max

#include "PurrfectEngine/transform.hpp"
#include "PurrfectEngine/camera.hpp"
#include "PurrfectEngine/scene.hpp"

namespace PurrfectEngine {

}

#include "PurrfectEngine/renderer.hpp"
#include "PurrfectEngine/input.hpp"
#include "PurrfectEngine/app.hpp"

#ifdef PURRENGINE_MAIN
#include "PurrfectEngine/entry.hpp"
#endif

#endif // PURRENGINE_PURRFECTENGINE_HPP_