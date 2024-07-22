#ifndef   PURRENGINE_PURRFECTENGINE_HPP_
#define   PURRENGINE_PURRFECTENGINE_HPP_

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE

#undef max

#include <string.h>
#include <vector>

#include "PurrfectEngine/extensions.hpp"

#include "PurrfectEngine/transform.hpp"
#include "PurrfectEngine/camera.hpp"
#include "PurrfectEngine/scene.hpp"
#include "PurrfectEngine/audio.hpp"

namespace PurrfectEngine {

  // https://stackoverflow.com/a/44159111: {
  template <class T>
  std::true_type is_public_base_of_impl(T*);

  template <class T>
  std::false_type is_public_base_of_impl(...);

  template <class B, class D>
  using is_public_base_of = decltype(is_public_base_of_impl<B>(std::declval<D*>()));
  // }

  bool vec_find_str(std::vector<const char *> vec, const char *str, size_t *pos);

}

#include "PurrfectEngine/utils.hpp"
#include "PurrfectEngine/renderer.hpp"
#include "PurrfectEngine/events.hpp"
#include "PurrfectEngine/input.hpp"
#include "PurrfectEngine/app.hpp"

#ifdef PURRENGINE_MAIN
#include "PurrfectEngine/entry.hpp"
#endif

#endif // PURRENGINE_PURRFECTENGINE_HPP_