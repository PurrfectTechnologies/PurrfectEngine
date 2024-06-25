#ifndef PURRFECTENGINE_AUDIO_HPP_
#define PURRFECTENGINE_AUDIO_HPP_

#include <AL/al.h>
#include <AL/alc.h>
#include <string>

namespace PurrfectEngine {

  class purrAudioEngine {
  public:
    purrAudioEngine();
    ~purrAudioEngine();

    bool initialize();
    void cleanup();

    bool loadSound(const std::string& filename, ALuint& buffer);
    bool playSound(ALuint buffer);
  private:
    ALCdevice  *mDevice = nullptr;
    ALCcontext *mContext = nullptr;
  };

}

#endif // PURRFECTENGINE_AUDIO_HPP_