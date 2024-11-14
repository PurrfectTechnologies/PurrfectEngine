#ifndef PURRENGINE_AUDIO_HPP_
#define PURRENGINE_AUDIO_HPP_

#ifndef AL_LIBTYPE_STATIC
#define AL_LIBTYPE_STATIC
#endif

#include <AL/al.h>
#include <AL/alc.h>
#include <AL/alext.h>
#include <AL/efx.h>
#include <AL/efx-creative.h>
#include <AL/efx-presets.h>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <thread>
#include <atomic>

class purrAudioFilter;

namespace PurrfectEngine {

  // typedef void (AL_APIENTRY *LPALGENAUXILIARYEFFECTSLOTS)(ALsizei, ALuint*);
  // typedef void (AL_APIENTRY *LPALDELETEAUXILIARYEFFECTSLOTS)(ALsizei, const ALuint*);
  // typedef void (AL_APIENTRY *LPALAUXILIARYEFFECTSLOTI)(ALuint, ALenum, ALint);
  // typedef void (AL_APIENTRY *LPALGENEFFECTS)(ALsizei, ALuint*);
  // typedef void (AL_APIENTRY *LPALDELETEEFFECTS)(ALsizei, const ALuint*);
  // typedef void (AL_APIENTRY *LPALEFFECTI)(ALuint, ALenum, ALint);
  // typedef void (AL_APIENTRY *LPALEFFECTF)(ALuint, ALenum, ALfloat);

  // extern LPALGENAUXILIARYEFFECTSLOTS alGenAuxiliaryEffectSlots;
  // extern LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots;
  // extern LPALAUXILIARYEFFECTSLOTI alAuxiliaryEffectSloti;
  // extern LPALGENEFFECTS alGenEffects;
  // extern LPALDELETEEFFECTS alDeleteEffects;
  // extern LPALEFFECTI alEffecti;
  // extern LPALEFFECTF alEffectf;

  class purrAudioSource {
    friend class purrAudioEngine;
  public:
    ~purrAudioSource();

    bool initialize();
    void cleanup();

    // bool copy();

    void play();
    void pause();
    void resume();
    void stop();
    void replay();

    void setTransform(purrTransform *transform) { mTransform = transform; }

    void update();
  private:
    purrAudioSource(purrTransform *transform);
  private:
    ALuint mSource = 0;
    ALuint mBuffer = 0;
    purrTransform *mTransform = nullptr;
  };

  class purrAudioListener {
  public:
    purrAudioListener(purrTransform *transform);
    ~purrAudioListener();

    void setTransform(purrTransform *transform) { mTransform = transform; }

    void update();
  private:
    purrTransform *mTransform = nullptr;
  };

  class purrAudioEngine {
  public:
    ~purrAudioEngine();

    bool initialize();
    void cleanup();

    static bool load(const char *filename, purrAudioSource **source);

    void update();

    void setListener(purrAudioListener *listener);
    purrAudioSource *newSource(purrTransform *transform = nullptr);
    bool deleteSource(purrAudioSource *source);
  public:
    inline static purrAudioEngine* getInstance() { return sInstance; }
    static bool create();
  private:
    purrAudioEngine();
  private:
    ALCdevice  *mDevice  = nullptr;
    ALCcontext *mContext = nullptr;

    purrAudioListener *mListener = nullptr;
    std::vector<purrAudioSource*> mSources{};

    // bool   mEFXSupported  = false;
    // ALuint mReverbEffect  = 0;
    // ALuint mAuxEffectSlot = 0;
  private:
    // bool loadEFXExtensionFunctions();
  private:
    inline static purrAudioEngine *sInstance = nullptr;
  };

  // class purrAudioControl {
  // public:
  //   void setListenerPosition(float x, float y, float z);
  //   void setListenerOrientation(float atX, float atY, float atZ, float upX, float upY, float upZ);
  //   void setSoundPosition(ALuint source, float x, float y, float z);
  //   void setDirection(ALuint source, float x, float y, float z);
  //   void setVelocity(ALuint source, float x, float y, float z);
  //   void setReverb(ALuint source, float gain);
  //   void setPitch(ALuint source, float pitch);
  //   void setGain(ALuint source, float gain);
  //   void playSoundFromTime(ALuint source, float seconds);
  //   void fadeIn(ALuint source, float duration);
  //   void fadeOut(ALuint source, float duration);
  //   void setLooping(ALuint source, bool loop);
  //   ALint getSourceState(ALuint source);
  //   void setDopplerFactor(float factor);
  //   void setSpeedOfSound(float speed);
  //   void setStereoPan(ALuint source, float pan);

  //   // set for equalizer (no no touchy)
  //   // void setBassLevel(ALuint source, float level);
  //   // void setMidLevel(ALuint source, float level);
  //   // void setTrebleLevel(ALuint source, float level);
  //   void applyEchoEffect(ALuint source, float delay, float feedback);
  //   void applyFlangEffect(ALuint source, float rate, float depth);
  // private:
  //   ALuint mReverbEffect  = 0;
  //   ALuint mAuxEffectSlot = 0;
  // };
}

#endif // PURRENGINE_AUDIO_HPP_