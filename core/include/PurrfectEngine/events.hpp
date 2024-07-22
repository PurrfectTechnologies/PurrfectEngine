#ifndef   PURRENGINE_EVENTS_HPP_
#define   PURRENGINE_EVENTS_HPP_

namespace PurrfectEngine {

  class purrEvent {
  public:
    purrEvent(const char *name);
  public:
    const char *getName() const { return mName; }
  private:
    const char *mName = nullptr;
  };

  class purrEventCancellable: public purrEvent {
  public:
    void setCancelled(bool cancelled = true);
    bool isCancelled();
  private:
    bool mCancelled = false;
  };

  template <typename T>
  using purr_event_callback_func = void (*)(T*);

  template <typename T>
  struct purrEventCallbacks {
    std::vector<const char *> names;
    std::vector<std::vector<purr_event_callback_func<T>>> callbacks;
  };

  template <typename T>
  class purrEventHandler {
  public:
    purrEventHandler() {
      static_assert(is_public_base_of<purrEvent, T>(), "purrEvent must be public base of T!");
    }

    void on(const char *name, purr_event_callback_func<T> callback) {
      size_t pos = mCallbacks.names.size();
      bool found = vec_find_str(mCallbacks.names, name, &pos);
      if (!found) {
        mCallbacks.names.push_back(name);
        mCallbacks.callbacks.push_back(std::vector<purr_event_callback_func<T>>());
      }
      mCallbacks.callbacks[pos].push_back(callback);
    }

    void once(const char *name, purr_event_callback_func<T> callback) {
      size_t pos = mOnceCallbacks.names.size();
      bool found = vec_find_str(mOnceCallbacks.names, name, &pos);
      if (!found) {
        mOnceCallbacks.names.push_back(name);
        mOnceCallbacks.callbacks.push_back(std::vector<purr_event_callback_func<T>>());
      }
      mOnceCallbacks.callbacks[pos].push_back(callback);
    }

    void fire(T *event) {
      fire(mCallbacks, event);
      fire(mOnceCallbacks, event, true);
    }
  private:
    void fire(purrEventCallbacks<T> &cbs, T *event, bool clear = false) {
      size_t pos = 0;
      bool found = false;
      if (cbs.names.size() <= 0 || !(found = vec_find_str(cbs.names, event->getName(), &pos))) return;
      for (purr_event_callback_func<T> cb: cbs.callbacks[pos]) cb(event);
      if (clear) cbs.callbacks[pos].clear();
    }
  private:
    purrEventCallbacks<T> mCallbacks{};
    purrEventCallbacks<T> mOnceCallbacks{};
  };

}

#endif // PURRENGINE_EVENTS_HPP_