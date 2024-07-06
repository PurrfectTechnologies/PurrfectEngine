#ifndef PURRFECTENGINE_AUDIO_HPP_
#define PURRFECTENGINE_AUDIO_HPP_

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

class purrAudioFilter;

namespace PurrfectEngine {

  typedef void (AL_APIENTRY *LPALGENAUXILIARYEFFECTSLOTS)(ALsizei, ALuint*);
  typedef void (AL_APIENTRY *LPALDELETEAUXILIARYEFFECTSLOTS)(ALsizei, const ALuint*);
  typedef void (AL_APIENTRY *LPALAUXILIARYEFFECTSLOTI)(ALuint, ALenum, ALint);
  typedef void (AL_APIENTRY *LPALGENEFFECTS)(ALsizei, ALuint*);
  typedef void (AL_APIENTRY *LPALDELETEEFFECTS)(ALsizei, const ALuint*);
  typedef void (AL_APIENTRY *LPALEFFECTI)(ALuint, ALenum, ALint);
  typedef void (AL_APIENTRY *LPALEFFECTF)(ALuint, ALenum, ALfloat);

  extern LPALGENAUXILIARYEFFECTSLOTS alGenAuxiliaryEffectSlots;
  extern LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots;
  extern LPALAUXILIARYEFFECTSLOTI alAuxiliaryEffectSloti;
  extern LPALGENEFFECTS alGenEffects;
  extern LPALDELETEEFFECTS alDeleteEffects;
  extern LPALEFFECTI alEffecti;
  extern LPALEFFECTF alEffectf;

  class purrAudioEngine: purrExtendable<purrAudioExt> {
  public:
    static purrAudioEngine& instance() {
      static purrAudioEngine instance;
      return instance;
    }

    purrAudioEngine(const purrAudioEngine&) = delete;
    purrAudioEngine& operator=(const purrAudioEngine&) = delete;

    bool initialize();
    void cleanup();

    bool load(const std::string& filename, ALuint& buffer);
    bool play(ALuint buffer);
    void pause(ALuint source);
    void resume(ALuint source);
    void stop(ALuint source);
    void replay(ALuint source);

    void addFilter(const std::string& name, std::shared_ptr<purrAudioFilter> filter);
    void applyFilter(const std::string& name, ALuint source);
    std::shared_ptr<purrAudioFilter> getFilter(const std::string& name);

    friend class purrAudioControl;
  private:
    purrAudioEngine();
    ~purrAudioEngine();

    ALCdevice  *mDevice = nullptr;
    ALCcontext *mContext = nullptr;

    bool mEFXSupported = false;
    ALuint mReverbEffect = 0;
    ALuint mEffectSlot = 0;

    std::map<std::string, std::shared_ptr<purrAudioFilter>> mFilters;

    bool loadEFXExtensionFunctions();
  };

  class purrAudioFilter {
  public:
    virtual ~purrAudioFilter() = default;

    virtual bool initialize() = 0;
    virtual void apply(ALuint source) = 0;
    virtual void setParameter(const std::string& param, float value) = 0;

  protected:
    ALuint mFilter;
  };

  class purrAudioControl {
  public: 
    static void setListenerPosition(float x, float y, float z);
    static void setListenerOrientation(float atX, float atY, float atZ, float upX, float upY, float upZ);
    static void setSoundPosition(ALuint source, float x, float y, float z);
    static void setDirection(ALuint source, float x, float y, float z);
    static void setVelocity(ALuint source, float x, float y, float z);
    static void setReverb(ALuint source, float density, float diffusion, float gain, float gainHF, float decayTime);
    static void setPitch(ALuint source, float pitch);
    static void setGain(ALuint source, float gain);
    static void playSoundFromTime(ALuint source, float seconds);
    static void fadeIn(ALuint source, float duration);
    static void fadeOut(ALuint source, float duration);
    static void setLooping(ALuint source, bool loop);
    static ALint getSourceState(ALuint source);
    static void setDopplerFactor(float factor);
    static void setSpeedOfSound(float speed);
    static void setStereoPan(ALuint source, float pan);

    static void addFilter(const std::string& name, std::shared_ptr<purrAudioFilter> filter);
    static void applyFilter(const std::string& name, ALuint source);


    // set for equalizer (no no touchy)
    static void setBassLevel(ALuint source, float level);
    static void setMidLevel(ALuint source, float level);
    static void setTrebleLevel(ALuint source, float level);
    static void applyEchoEffect(ALuint source, float delay, float feedback);
    static void applyFlangEffect(ALuint source, float rate, float depth);
  };
}

#endif // PURRFECTENGINE_AUDIO_HPP_