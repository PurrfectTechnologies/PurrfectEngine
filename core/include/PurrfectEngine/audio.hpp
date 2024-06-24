#ifndef PURRFECTENGINE_AUDIO_HPP_
#define PURRFECTENGINE_AUDIO_HPP_

#include <AL/al.h>
#include <AL/alc.h>
#include <string>

namespace PurrfectEngine {
  class AudioEngine {
  public:
    AudioEngine();
    ~AudioEngine();

    void init();
    void loadSound(const std::string& filename, ALuint& buffer);
    void playSound(ALuint buffer);

  private:
    ALCdevice* device;
    ALCcontext* context;
  };
}

#endif // PURRFECTENGINE_AUDIO_HPP_