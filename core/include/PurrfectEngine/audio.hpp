#ifndef PURRFECTENGINE_AUDIO_HPP_
#define PURRFECTENGINE_AUDIO_HPP_

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <AL/auxeffectslot.h>
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
    void setListenerPosition(float x, float y, float z);
    void setListenerOrientation(float atX, float atY, float atZ, float upX, float upY, float upZ);
    void setSoundPosition(ALuint source, float x, float y, float z);
    void setReverb(ALuint source, float density, float diffusion, float gain, float gainHF, float decayTime);
    void setBass(ALuint source, float bassLevel);
    void setGain(ALuint source, float gain);
  private:
    ALCdevice  *mDevice = nullptr;
    ALCcontext *mContext = nullptr;

    bool mEFXSupported = false;
    ALuint mReverbEffect = 0;
    ALuint mAuxEffectSlot = 0;
  };

}

#endif // PURRFECTENGINE_AUDIO_HPP_