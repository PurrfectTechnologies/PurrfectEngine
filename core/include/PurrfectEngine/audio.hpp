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
#include <iostream>

namespace PurrfectEngine {

  class purrAudioEngine: purrExtendable<purrAudioExt> {
  public:
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

    void setListenerPosition(float x, float y, float z);
    void setListenerOrientation(float atX, float atY, float atZ, float upX, float upY, float upZ);
    void setSoundPosition(ALuint source, float x, float y, float z);
    void setReverb(ALuint source, float density, float diffusion, float gain, float gainHF, float decayTime);
    void setPitch(ALuint source, float pitch);
    void setGain(ALuint source, float gain);
    void playSoundFromTime(ALuint source, float seconds);

    
  private:
    ALCdevice  *mDevice = nullptr;
    ALCcontext *mContext = nullptr;

    bool mEFXSupported = false;
    ALuint mReverbEffect = 0;
    ALuint mEffectSlot = 0;

    bool loadEFXExtensionFunctions();
  
  private:
    typedef void (AL_APIENTRY *LPALGENAUXILIARYEFFECTSLOTS)(ALsizei, ALuint*);
    typedef void (AL_APIENTRY *LPALDELETEAUXILIARYEFFECTSLOTS)(ALsizei, const ALuint*);
    typedef void (AL_APIENTRY *LPALAUXILIARYEFFECTSLOTI)(ALuint, ALenum, ALint);
    typedef void (AL_APIENTRY *LPALGENEFFECTS)(ALsizei, ALuint*);
    typedef void (AL_APIENTRY *LPALDELETEEFFECTS)(ALsizei, const ALuint*);
    typedef void (AL_APIENTRY *LPALEFFECTI)(ALuint, ALenum, ALint);
    typedef void (AL_APIENTRY *LPALEFFECTF)(ALuint, ALenum, ALfloat);

    LPALGENAUXILIARYEFFECTSLOTS alGenAuxiliaryEffectSlots = nullptr;
    LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots = nullptr;
    LPALAUXILIARYEFFECTSLOTI alAuxiliaryEffectSloti = nullptr;
    LPALGENEFFECTS alGenEffects = nullptr;
    LPALDELETEEFFECTS alDeleteEffects = nullptr;
    LPALEFFECTI alEffecti = nullptr;
    LPALEFFECTF alEffectf = nullptr;
  };

}

#endif // PURRFECTENGINE_AUDIO_HPP_