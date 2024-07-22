#include "PurrfectEngine/PurrfectEngine.hpp"

namespace PurrfectEngine {

  purrEvent::purrEvent(const char *name):
    mName(name)
  {}

  void purrEventCancellable::setCancelled(bool cancelled) {
    mCancelled = cancelled;
  }

  bool purrEventCancellable::isCancelled() {
    return mCancelled;
  }

}