#ifndef   PURRENGINE_ENTRY_HPP_
#define   PURRENGINE_ENTRY_HPP_

namespace PurrfectEngine {
  namespace Entry {

    int main(int argc, char **argv) {
      purrApp *app = CreateApp();
      if (!app->init()) return 1;
      app->run();
      delete app;
      return 0;
    }

  }
}

int main(int argc, char **argv) {
  return PurrfectEngine::Entry::main(argc, argv);
}

#endif // PURRENGINE_ENTRY_HPP_