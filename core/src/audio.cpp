#define MINIMP3_IMPLEMENTATION

#include "PurrfectEngine/PurrfectEngine.hpp"

#include <iostream>
#include <vector>
#include "minimp3_ex.h"
#include <sndfile.h>
#include <thread>
#include <chrono>

namespace PurrfectEngine {

  // LPALGENAUXILIARYEFFECTSLOTS alGenAuxiliaryEffectSlots = nullptr;
  // LPALDELETEAUXILIARYEFFECTSLOTS alDeleteAuxiliaryEffectSlots = nullptr;
  // LPALAUXILIARYEFFECTSLOTI alAuxiliaryEffectSloti = nullptr;
  // LPALGENEFFECTS alGenEffects = nullptr;
  // LPALDELETEEFFECTS alDeleteEffects = nullptr;
  // LPALEFFECTI alEffecti = nullptr;
  // LPALEFFECTF alEffectf = nullptr;

  purrAudioListener::purrAudioListener(purrTransform *transform):
    mTransform(transform)
  {}

  purrAudioListener::~purrAudioListener() {

  }

  void purrAudioListener::update() {
    glm::vec3 pos = mTransform->getPosition();
    alListener3f(AL_POSITION, pos.x, pos.y, pos.z);
    alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f);
    glm::vec3 at = mTransform->getPosition() + mTransform->getForward();
    glm::vec3 up = mTransform->getPosition() + mTransform->getUp();
    ALfloat listenerOri[] =  {at.x, at.y, at.z, up.x, up.y, up.z};
    alListenerfv(AL_ORIENTATION, listenerOri);
  }

  purrAudioSource::purrAudioSource(purrTransform *transform):
    mTransform(transform)
  {}

  purrAudioSource::~purrAudioSource() {
    cleanup();
  }
  
  bool purrAudioSource::initialize() {
    if (mSource != 0) return false;
    alGenSources(1, &mSource);

    if (mBuffer != 0) alSourcei(mSource, AL_BUFFER, mBuffer);
    alSourcef(mSource,  AL_PITCH, 1);
    alSourcef(mSource,  AL_GAIN, 1);

    glm::vec3 pos = ((mTransform)?mTransform->getPosition():glm::vec3(0.0f));
    alSource3f(mSource, AL_POSITION, pos.x, pos.y, pos.z);
    alSource3f(mSource, AL_VELOCITY, 0, 0, 0);
    alSourcei(mSource,  AL_LOOPING, AL_FALSE);

    return true;
  }
  
  void purrAudioSource::cleanup() {
    if (mSource != 0) alDeleteSources(1, &mSource);
    if (mBuffer != 0) alDeleteBuffers(1, &mBuffer);
  }
  
  // TODO: implement
  // bool purrAudioSource::copy() {

  // }
  
  void purrAudioSource::play() {
    if (mSource != 0 && mBuffer != 0) alSourcePlay(mSource);
  }
  
  void purrAudioSource::pause() {
    if (mSource != 0) alSourcePause(mSource);
  }
  
  void purrAudioSource::resume() {
    if (mSource == 0) return;
    ALint state = 0;
    alGetSourcei(mSource, AL_SOURCE_STATE, &state);
    if (state == AL_PAUSED) alSourcePlay(mSource);
  }
  
  void purrAudioSource::stop() {
    if (mSource != 0) alSourceStop(mSource);
  }
  
  void purrAudioSource::replay() {
    if (mSource == 0) return;
    alSourceRewind(mSource);
    alSourcePlay(mSource);
  }
  
  void purrAudioSource::update() {
    if (mSource == 0 || !mTransform) return;
    glm::vec3 pos = mTransform->getPosition();
    alSource3f(mSource, AL_POSITION, pos.x, pos.y, pos.z);
    alSource3f(mSource, AL_VELOCITY, 0, 0, 0);
  }

  purrAudioEngine::purrAudioEngine()
  {}

  purrAudioEngine::~purrAudioEngine() {
    cleanup();
  }

  bool purrAudioEngine::initialize() {
    mDevice = alcOpenDevice(nullptr);
    if (!mDevice) return false;

    mContext = alcCreateContext(mDevice, nullptr);
    if (!alcMakeContextCurrent(mContext)) return false;

    // if (alcIsExtensionPresent(mDevice, "ALC_EXT_EFX")) {
    //   mEFXSupported = true;
    //   if (!loadEFXExtensionFunctions()) {
    //     std::cerr << "Failed to load EFX extension functions!" << std::endl;
    //     return false;
    //   }
    //   alGenAuxiliaryEffectSlots(1, &mAuxEffectSlot);
    //   alGenEffects(1, &mReverbEffect);
    // } else {
    //   mEFXSupported = false;
    //   std::cerr << "EFX not supported! Exiting." << std::endl;
    //   cleanup();
    //   return false;
    // }

    return true;
  }

  void purrAudioEngine::cleanup() {
    if (mListener) delete mListener;
    for (purrAudioSource *source: mSources) delete source;

    // if (mAuxEffectSlot != 0) {
    //   alDeleteAuxiliaryEffectSlots(1, &mAuxEffectSlot);
    // }
    // if (mReverbEffect != 0) {
    //   alDeleteEffects(1, &mReverbEffect);
    // }
    alcMakeContextCurrent(nullptr);
    if (mContext != nullptr) alcDestroyContext(mContext);
    if (mDevice != nullptr)  alcCloseDevice(mDevice);
  }

  void purrAudioEngine::update() {
    if (!mListener) return;
    mListener->update();
    for (purrAudioSource *source: mSources) source->update();

    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }

  bool purrAudioEngine::load(const char *filename, purrAudioSource **source) {
    assert(sInstance && sInstance->mContext);
    if (!source || !(*source)) return false;

    const char *ext = NULL;
    { // Find extension of filename
      const char *ptr = strrchr(filename, '.');
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
      format = ((info.channels == 1)?AL_FORMAT_MONO16:AL_FORMAT_STEREO16);
      sampleRate = info.hz;
    } else {
      SF_INFO sfInfo;
      SNDFILE *sndFile = sf_open(filename, SFM_READ, &sfInfo);
      if (!sndFile) return false;

      std::vector<short> samplesV(sfInfo.frames * sfInfo.channels);
      sf_read_short(sndFile, samplesV.data(), sfInfo.frames * sfInfo.channels);
      sf_close(sndFile);

      if (sfInfo.channels < 1 || sfInfo.channels > 2) return false;
      format = ((sfInfo.channels == 1)?AL_FORMAT_MONO16:AL_FORMAT_STEREO16);
      sampleRate = sfInfo.samplerate;
      sampleCount = samplesV.size();
      samples = (short*)malloc(sampleCount*sizeof(short));
      memcpy(samples, samplesV.data(), sampleCount*sizeof(short));
    }

    alGenBuffers(1, &(*source)->mBuffer);
    alBufferData((*source)->mBuffer, format, samples, sampleCount*sizeof(*samples), sampleRate);
    free(samples);

    return true;
  }

  void purrAudioEngine::setListener(purrAudioListener *listener) {
    if (mListener) delete mListener;
    mListener = listener;
  }

  purrAudioSource *purrAudioEngine::newSource(purrTransform *transform) {
    purrAudioSource *source = new purrAudioSource(transform);
    mSources.push_back(source);
    return source;
  }

  bool purrAudioEngine::deleteSource(purrAudioSource *source) {
    if (auto it = std::find(mSources.begin(), mSources.end(), source); it != mSources.end()) {
      mSources.erase(it, mSources.end());
      delete *it;
    }
    else return false;
    return true;
  }

  bool purrAudioEngine::create() {
    if (sInstance) return false;
    sInstance = new purrAudioEngine();
    return true;
  }

  // bool purrAudioEngine::loadEFXExtensionFunctions() {
  //   alGenAuxiliaryEffectSlots = (LPALGENAUXILIARYEFFECTSLOTS)alGetProcAddress("alGenAuxiliaryEffectSlots");
  //   alDeleteAuxiliaryEffectSlots = (LPALDELETEAUXILIARYEFFECTSLOTS)alGetProcAddress("alDeleteAuxiliaryEffectSlots");
  //   alAuxiliaryEffectSloti = (LPALAUXILIARYEFFECTSLOTI)alGetProcAddress("alAuxiliaryEffectSloti");
  //   alGenEffects = (LPALGENEFFECTS)alGetProcAddress("alGenEffects");
  //   alDeleteEffects = (LPALDELETEEFFECTS)alGetProcAddress("alDeleteEffects");
  //   alEffecti = (LPALEFFECTI)alGetProcAddress("alEffecti");
  //   alEffectf = (LPALEFFECTF)alGetProcAddress("alEffectf");

  //   return alGenAuxiliaryEffectSlots && alDeleteAuxiliaryEffectSlots && alAuxiliaryEffectSloti
  //       && alGenEffects && alDeleteEffects && alEffecti && alEffectf;
  // }

  // void purrAudioControl::fadeIn(ALuint source, float duration) {
  //   alSourcef(source, AL_GAIN, 0.0f);
  //   alSourcePlay(source);
  //   float gain = 0.0f;
  //   float increment = 1.0f / (duration * 10);
  //   for (int i = 0; i < duration * 10; ++i) {
  //     gain += increment;
  //     if (gain > 1.0f) gain = 1.0f;
  //     alSourcef(source, AL_GAIN, gain);
  //     std::this_thread::sleep_for(std::chrono::milliseconds(100));
  //   };
  // }

  // void purrAudioControl::fadeOut(ALuint source, float duration) {
  //   float gain;
  //   alGetSourcef(source, AL_GAIN, &gain);
  //   float decrement = gain / (duration * 10);
  //   for (int i = 0; i < duration * 10; ++i) {
  //     gain -= decrement;
  //     if (gain < 0.0f) gain = 0.0f;
  //     alSourcef(source, AL_GAIN, gain);
  //     std::this_thread::sleep_for(std::chrono::milliseconds(100));
  //   }
  //   alSourceStop(source);
  // }
}