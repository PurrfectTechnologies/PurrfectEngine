#ifndef PURRENGINE_AUDIO_HPP_
#define PURRENGINE_AUDIO_HPP_

#define AL_LIBTYPE_STATIC

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
  
  class purrAudioExt {
  public:
    virtual bool initialize() = 0;
    virtual bool preUpdate()  = 0;
    virtual bool update()     = 0;
    virtual void cleanup()    = 0;
  };

  class purrAudioFilter : public purrAudioExt {
  public:
    virtual ~purrAudioFilter() = default;

    virtual bool initialize() override = 0;
    virtual bool preUpdate() override { return true; }
    virtual bool update() override { return true; }
    virtual void cleanup() override = 0;
    virtual void apply(ALuint source) = 0;
    virtual void setParameter(const std::string& param, float value) = 0;

  protected:
    ALuint mFilter;
  };

  class purrAudioEngine: purrExtendable<purrAudioExt> {
  public:
    static purrAudioEngine& instance();

    
    purrAudioEngine(std::vector<purrAudioExt*> extensions);
    
    purrAudioEngine();
    ~purrAudioEngine();

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
  public:
  static purrAudioEngine *getInstance() { return sInstance; }
  
  private:
    purrAudioEngine(const purrAudioEngine&) = delete;
    purrAudioEngine& operator=(const purrAudioEngine&) = delete;

    inline static purrAudioEngine *sInstance = nullptr;

    ALCdevice  *mDevice = nullptr;
    ALCcontext *mContext = nullptr;

    bool mEFXSupported = false;
    ALuint mReverbEffect = 0;
    ALuint mAuxEffectSlot = 0;

    std::map<std::string, std::shared_ptr<purrAudioFilter>> mFilters;

    bool loadEFXExtensionFunctions();
  };

  class purrAudioControl {
  public: 
    void setListenerPosition(float x, float y, float z);
    void setListenerOrientation(float atX, float atY, float atZ, float upX, float upY, float upZ);
    void setSoundPosition(ALuint source, float x, float y, float z);
    void setDirection(ALuint source, float x, float y, float z);
    void setVelocity(ALuint source, float x, float y, float z);
    void setReverb(ALuint source, float gain);
    void setPitch(ALuint source, float pitch);
    void setGain(ALuint source, float gain);
    void playSoundFromTime(ALuint source, float seconds);
    void fadeIn(ALuint source, float duration);
    void fadeOut(ALuint source, float duration);
    void setLooping(ALuint source, bool loop);
    ALint getSourceState(ALuint source);
    void setDopplerFactor(float factor);
    void setSpeedOfSound(float speed);
    void setStereoPan(ALuint source, float pan);

    void addFilter(const std::string& name, std::shared_ptr<purrAudioFilter> filter);
    void applyFilter(const std::string& name, ALuint source);


    // set for equalizer (no no touchy)
    // void setBassLevel(ALuint source, float level);
    // void setMidLevel(ALuint source, float level);
    // void setTrebleLevel(ALuint source, float level);
    void applyEchoEffect(ALuint source, float delay, float feedback);
    void applyFlangEffect(ALuint source, float rate, float depth);

  private:
    ALuint mReverbEffect = 0;
    ALuint mAuxEffectSlot = 0;
  };
}

#endif // PURRENGINE_AUDIO_HPP_