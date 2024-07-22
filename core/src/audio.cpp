#define MINIMP3_IMPLEMENTATION

#include "PurrfectEngine/PurrfectEngine.hpp"

#include <iostream>
#include <vector>
#include "minimp3_ex.h"
#include <sndfile.h>
#include <thread>
#include <chrono>

namespace PurrfectEngine {

  LPALGENAUXILIARYEFFECTSLOTS alGenAuxiliaryEffectSlots = nullptr;
  LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots = nullptr;
  LPALAUXILIARYEFFECTSLOTI alAuxiliaryEffectSloti = nullptr;
  LPALGENEFFECTS alGenEffects = nullptr;
  LPALDELETEEFFECTS alDeleteEffects = nullptr;
  LPALEFFECTI alEffecti = nullptr;
  LPALEFFECTF alEffectf = nullptr;

  purrAudioEngine::purrAudioEngine(std::vector<purrAudioExt*> extensions):
    purrExtendable<purrAudioExt>(extensions), mDevice(nullptr), mContext(nullptr), mReverbEffect(0), mAuxEffectSlot(0)
  {}

  purrAudioEngine::~purrAudioEngine() {
    cleanup();
  }

  bool purrAudioEngine::initialize() {
    if (mRunning) return false;
    mDevice = alcOpenDevice(nullptr);
    if (!mDevice) return false;

    mContext = alcCreateContext(mDevice, nullptr);
    if (!alcMakeContextCurrent(mContext)) return false;

    alListener3f(AL_POSITION, 0.0f, 0.0f,  0.0f);
    alListener3f(AL_VELOCITY, 0.0f, 0.0f,  0.0f);
    ALfloat listenerOri[] =  {0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f};
    alListenerfv(AL_ORIENTATION, listenerOri);

    if (!loadEFXExtensionFunctions()) {
      std::cerr << "Failed to load EFX extension functions!" << std::endl;
      return false;
    }

    if (alcIsExtensionPresent(mDevice, "ALC_EXT_EFX")) {
      mEFXSupported = true;
      alGenAuxiliaryEffectSlots(1, &mAuxEffectSlot);
      alGenEffects(1, &mReverbEffect);
    } else {
      mEFXSupported = false;
      std::cerr << "EFX not supported! Exiting." << std::endl;
      cleanup();
      return false;
    }

    mRunning = true;
    mThread = std::thread(&purrAudioEngine::run, this);
    return true;
  }

  void purrAudioEngine::cleanup() {
    if (!mRunning) return;

    mRunning = false;
    if (mThread.joinable()) {
      mThread.join();
    } 
    if (mAuxEffectSlot != 0) {
      alDeleteAuxiliaryEffectSlots(1, &mAuxEffectSlot);
    }
    if (mReverbEffect != 0) {
      alDeleteEffects(1, &mReverbEffect);
    }
    alcMakeContextCurrent(nullptr);
    if (mContext != nullptr) {
      alcDestroyContext(mContext);
    }
    if (mDevice != nullptr) {
      alcCloseDevice(mDevice);
    }
  }

  void purrAudioEngine::run() {
    while (mRunning) {
      for (auto& [name, filter] : mFilters) {
        filter->update();
      }
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  }

  bool purrAudioEngine::load(char *filename, ALuint buffer) {
    char *ext = NULL;
    { // Find extension of filename
      char *ptr = strrchr(filename, '.');
      if (!ptr) return false;
      ext = ptr+1;
    }

    int    sampleRate  = 0;
    short *samples     = NULL;
    size_t sampleCount = 0;
    ALenum format = 0;
    if (strcmp(ext, "mp3") == 0) {
      mp3dec_t mp3d;
      mp3dec_file_info_t info;
      if (mp3dec_load(&mp3d, filename, &info, nullptr, nullptr)) return false;
      if (info.channels < 1 || info.channels > 2) return false;
      format = (info.channels == 1)?AL_FORMAT_MONO16:AL_FORMAT_STEREO16;
      sampleRate = info.hz;
    } else {
      SF_INFO sfInfo;
      SNDFILE *sndFile = sf_open(filename, SFM_READ, &sfInfo);
      if (!sndFile) return false;

      std::vector<short> samplesV(sfInfo.frames * sfInfo.channels);
      sf_read_short(sndFile, samplesV.data(), sfInfo.frames * sfInfo.channels);
      sf_close(sndFile);

      if (sfInfo.channels < 1 || sfInfo.channels > 2) return false;
      format = (sfInfo.channels == 1)?AL_FORMAT_MONO16:AL_FORMAT_STEREO16;
      sampleRate = sfInfo.samplerate;
      sampleCount = samplesV.size();
      memcpy(samples, samplesV.data(), sampleCount*sizeof(short));
    }

    alGenBuffers(1, &buffer);
    alBufferData(buffer, format, samples, sampleCount * sizeof(*samples), sampleRate);
    free(samples);

    return true;
  }

  void purrAudioEngine::play(ALuint buffer) {
    ALuint source;
    alGenSources(1, &source);

    alSourcei(source, AL_BUFFER, buffer);
    alSourcef(source, AL_PITCH, 1);
    alSourcef(source, AL_GAIN, 1);
    alSource3f(source, AL_POSITION, 0, 0, 0);
    alSource3f(source, AL_VELOCITY, 0, 0, 0);
    alSourcei(source, AL_LOOPING, AL_FALSE);

    if (mEFXSupported) alSource3i(source, AL_AUXILIARY_SEND_FILTER, mAuxEffectSlot, 0, AL_FILTER_NULL);

    alSourcePlay(source);

    ALint state;
    do {
      alGetSourcei(source, AL_SOURCE_STATE, &state);
    } while (state == AL_PLAYING);

    alDeleteSources(1, &source);
  }

  void purrAudioEngine::pause(ALuint source) {
    alSourcePause(source);
  }

  void purrAudioEngine::resume(ALuint source) {
    ALint state = 0;
    alGetSourcei(source, AL_SOURCE_STATE, &state);
    if (state == AL_PAUSED) alSourcePlay(source);
  }

  void purrAudioEngine::stop(ALuint source) {
    alSourceStop(source);
    alSourcei(source, AL_BUFFER, 0);
  }

  void purrAudioEngine::replay(ALuint source) {
    alSourceRewind(source);
    alSourcePlay(source);
  }

  void purrAudioEngine::addFilter(const char *name, std::shared_ptr<purrAudioFilter> filter) {
    mFilters[name] = filter;
  }

  bool purrAudioEngine::applyFilter(const char *name, ALuint source) {
    return ((mFilters.find(name) == mFilters.end())?false:(mFilters[name]->apply(source), true));
  }

  std::shared_ptr<purrAudioFilter> purrAudioEngine::getFilter(const char *name) {
    return ((mFilters.find(name) != mFilters.end())?mFilters[name]:nullptr);
  }

  bool purrAudioEngine::loadEFXExtensionFunctions() {
    alGenAuxiliaryEffectSlots = (LPALGENAUXILIARYEFFECTSLOTS)alGetProcAddress("alGenAuxiliaryEffectSlots");
    alDeleteAuxiliaryEffectSlots = (LPALDELETEAUXILIARYEFFECTSLOTS)alGetProcAddress("alDeleteAuxiliaryEffectSlots");
    alAuxiliaryEffectSloti = (LPALAUXILIARYEFFECTSLOTI)alGetProcAddress("alAuxiliaryEffectSloti");
    alGenEffects = (LPALGENEFFECTS)alGetProcAddress("alGenEffects");
    alDeleteEffects = (LPALDELETEEFFECTS)alGetProcAddress("alDeleteEffects");
    alEffecti = (LPALEFFECTI)alGetProcAddress("alEffecti");
    alEffectf = (LPALEFFECTF)alGetProcAddress("alEffectf");

    return alGenAuxiliaryEffectSlots && alDeleteAuxiliaryEffectSlots && alAuxiliaryEffectSloti
        && alGenEffects && alDeleteEffects && alEffecti && alEffectf;
  }

  // purrAudioControl:
  void purrAudioControl::playSoundFromTime(ALuint source, float seconds) {
    alSourcef(source, AL_SEC_OFFSET, seconds);
    alSourcePlay(source);
  }

  void purrAudioControl::setListenerPosition(float x, float y, float z) {
    alListener3f(AL_POSITION, x, y, z);
  }

  void purrAudioControl::setListenerOrientation(float atX, float atY, float atZ, float upX, float upY, float upZ) {
    ALfloat listenerOri[] = {atX, atY, atZ, upX, upY, upZ};
    alListenerfv(AL_ORIENTATION, listenerOri);
  }

  void purrAudioControl::setSoundPosition(ALuint source, float x, float y, float z) {
    alSource3f(source, AL_POSITION, x, y, z);
  }

  void purrAudioControl::setVelocity(ALuint source, float x, float y, float z) {
    ALfloat velocity[] = {x, y, z};
    alSourcefv(source, AL_VELOCITY, velocity);
  }

  void purrAudioControl::setDirection(ALuint source, float x, float y, float z) {
    ALfloat direction[] = {x, y, z};
    alSourcefv(source, AL_DIRECTION, direction);
  }

  void purrAudioControl::setReverb(ALuint source, float gain) {
    alEffecti(mReverbEffect, AL_EFFECT_TYPE, AL_EFFECT_REVERB);
    alEffectf(mReverbEffect, AL_REVERB_GAIN, gain);
    alAuxiliaryEffectSloti(mAuxEffectSlot, AL_EFFECTSLOT_EFFECT, mReverbEffect);
  }

  void purrAudioControl::setPitch(ALuint source, float pitch) {
    alSourcef(source, AL_PITCH, pitch);
  }

  void purrAudioControl::setGain(ALuint source, float gain) {
    alSourcef(source, AL_GAIN, gain);
  }

  void purrAudioControl::fadeIn(ALuint source, float duration) {
    alSourcef(source, AL_GAIN, 0.0f);
    alSourcePlay(source);
    float gain = 0.0f;
    float increment = 1.0f / (duration * 10);
    for (int i = 0; i < duration * 10; ++i) {
      gain += increment;
      if (gain > 1.0f) gain = 1.0f;
      alSourcef(source, AL_GAIN, gain);
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    };
  }

  void purrAudioControl::fadeOut(ALuint source, float duration) {
    float gain;
    alGetSourcef(source, AL_GAIN, &gain);
    float decrement = gain / (duration * 10);
    for (int i = 0; i < duration * 10; ++i) {
      gain -= decrement;
      if (gain < 0.0f) gain = 0.0f;
      alSourcef(source, AL_GAIN, gain);
      std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    alSourceStop(source);
  }

  void purrAudioControl::setLooping(ALuint source, bool loop) {
    alSourcei(source, AL_LOOPING, loop ? AL_TRUE : AL_FALSE);
  }

  ALint purrAudioControl::getSourceState(ALuint source) {
    ALint state;
    alGetSourcei(source, AL_SOURCE_STATE, &state);
    return state;
  }

  void purrAudioControl::setDopplerFactor(float factor) {
    alDopplerFactor(factor);
  }

  void purrAudioControl::setSpeedOfSound(float speed) {
    alSpeedOfSound(speed);
  }

  void purrAudioControl::setStereoPan(ALuint source, float pan) {
    ALfloat sourcePos[] = {pan, 0.0f, 0.0f};
    alSourcefv(source, AL_POSITION, sourcePos);
  }

  void purrAudioControl::applyEchoEffect(ALuint source, float delay, float feedback) {
    ALuint effect;
    alGenEffects(1, &effect);
    alEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_ECHO);
    alEffectf(effect, AL_ECHO_DELAY, delay);
    alEffectf(effect, AL_ECHO_FEEDBACK, feedback);

    ALuint auxEffectSlot;
    alGenAuxiliaryEffectSlots(1, &auxEffectSlot);
    alAuxiliaryEffectSloti(auxEffectSlot, AL_EFFECTSLOT_EFFECT, effect);

    alSource3i(source, AL_AUXILIARY_SEND_FILTER, auxEffectSlot, 0, AL_FILTER_NULL);
  }

  void purrAudioControl::applyFlangEffect(ALuint source, float rate, float depth) {
    ALuint effect;
    alGenEffects(1, &effect);
    alEffecti(effect, AL_EFFECT_TYPE, AL_EFFECT_FLANGER);
    alEffectf(effect, AL_FLANGER_RATE, rate);
    alEffectf(effect, AL_FLANGER_DEPTH, depth);

    ALuint auxEffectSlot;
    alGenAuxiliaryEffectSlots(1, &auxEffectSlot);
    alAuxiliaryEffectSloti(auxEffectSlot, AL_EFFECTSLOT_EFFECT, effect);

    alSource3i(source, AL_AUXILIARY_SEND_FILTER, auxEffectSlot, 0, AL_FILTER_NULL);
  }
}