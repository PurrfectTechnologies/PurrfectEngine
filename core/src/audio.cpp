#include "PurrfectEngine/PurrfectEngine.hpp"

#include <iostream>
#include <vector>
#include "minimp3_ex.h"
#include <sndfile.h>

namespace PurrfectEngine {

  purrAudioEngine::purrAudioEngine():
    mDevice(nullptr), mContext(nullptr), mReverbEffect(0), mEffectSlot(0)
  {}

  purrAudioEngine::~purrAudioEngine() {
    cleanup();
  }

  

  bool loadEFXExtensionFunctions() {
    alGenAuxiliaryEffectSlots = (LPALGENAUXILIARYEFFECTSLOTS)alGetProcAddress("alGenAuxiliaryEffectSlots");
    alDeleteAuxiliaryEffectSlots = (LPALDELETEAUXILIARYEFFECTSLOTS)alGetProcAddress("alDeleteAuxiliaryEffectSlots");
    alAuxiliaryEffectSloti = (LPALAUXILIARYEFFECTSLOTI)alGetProcAddress("alAuxiliaryEffectSloti");
    alGenEffects = (LPALGENEFFECTS)alGetProcAddress("alGenEffects");
    alDeleteEffects = (LPALDELETEEFFECTS)alGetProcAddress("alDeleteEffects");
    alEffecti = (LPALEFFECTI)alGetProcAddress("alEffecti");
    alEffectf = (LPALEFFECTF)alGetProcAddress("alEffectf");

    return alGenAuxiliaryEffectSlots && alDeleteAuxiliaryEffectSlots && alAuxiliaryEffectSloti &&
          alGenEffects && alDeleteEffects && alEffecti && alEffectf;
  }

  bool purrAudioEngine::initialize() {
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
      alGenAuxiliaryEffectSlots(1, &mEffectSlot);
      alGenEffects(1, &mReverbEffect);
    } else {
      mEFXSupported = false;
      std::cerr << "EFX not supported! Exiting." << std::endl;
      cleanup();
      return false;
    }

    return true;
  }

  void purrAudioEngine::cleanup() {
    if (mEffectSlot != 0) {
      alDeleteAuxiliaryEffectSlots(1, &mEffectSlot);
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

  bool purrAudioEngine::load(const std::string &filename, ALuint &buffer) {
    std::string extension = filename.substr(filename.find_last_of(".") + 1);

    if (extension == "mp3") {
      mp3dec_t mp3d;
      mp3dec_file_info_t info;
      if (mp3dec_load(&mp3d, filename.c_str(), &info, nullptr, nullptr)) return false;
      ALenum format = info.channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
      alGenBuffers(1, &buffer);
      alBufferData(buffer, format, info.buffer, info.samples * sizeof(mp3d_sample_t), info.hz);

      free(info.buffer);
    } else {
      SF_INFO sfInfo;
      SNDFILE *sndFile = sf_open(filename.c_str(), SFM_READ, &sfInfo);
      if (!sndFile) return false;

      std::vector<short> samples(sfInfo.frames * sfInfo.channels);
      sf_read_short(sndFile, samples.data(), sfInfo.frames * sfInfo.channels);
      sf_close(sndFile);

      ALenum format;
      if (sfInfo.channels == 1) {
        format = AL_FORMAT_MONO16;
      } else if (sfInfo.channels == 2) {
        format = AL_FORMAT_STEREO16;
      } else return false;

      alGenBuffers(1, &buffer);
      alBufferData(buffer, format, samples.data(), samples.size() * sizeof(short), sfInfo.samplerate);
    }
    return true;
  }

  bool purrAudioEngine::play(ALuint buffer) {
    ALuint source;
    alGenSources(1, &source);

    alSourcei(source, AL_BUFFER, buffer);
    alSourcef(source, AL_PITCH, 1);
    alSourcef(source, AL_GAIN, 1);
    alSource3f(source, AL_POSITION, 0, 0, 0);
    alSource3f(source, AL_VELOCITY, 0, 0, 0);
    alSourcei(source, AL_LOOPING, AL_FALSE);

    if (mEFXSupported) {
      alSource3i(source, AL_AUXILIARY_SEND_FILTER, mAuxEffectSlot, 0, AL_FILTER_NULL);
    }

    alSourcePlay(source);

    ALint state;
    do {
      alGetSourcei(source, AL_SOURCE_STATE, &state);
    } while (state == AL_PLAYING);

    alDeleteSources(1, &source);
    return true;
  }

  void purrAudioEngine::pause(ALuint source) {
    alSourcePause(source);
  }

  void purrAudioEngine::resume(ALuint source) {
    ALint state;
    alGetSourcei(source, AL_SOURCE_STATE, &state);
    if (state = AL_PAUSED) {
      alSourcePlay(source)''
    }
  }

  void purrAudioEngine::stop(ALuint source) {
    alSourceStop(source);
    alSourcei(source, AL_BUFFER, 0);
  }

  void purrAudioEngine::replay(ALuint source) {
    alSourceRewind(source);
    alSourcePlay(source);
  }

  void purrAudioEngine::playSoundFromTime(ALuint source, float seconds) {
    alSourcef(source, AL_SEC_OFFSET, seconds);
    alSourcePlay(source);
  }

  void purrAudioEngine::setListenerPosition(float x, float y, float z) {
    alListener3f(AL_POSITION, x, y, z);
  }

  void purrAudioEngine::setListenerOrientation(float atX, float atY, float atZ, float upX, float upY, float upZ) {
    ALfloat listenerOri[] = {atX, atY, atZ, upX, upY, upZ};
    alListenerfv(AL_ORIENTATION, listenerOri);
  }

  void purrAudioEngine::setSoundPosition(ALuint source, float x, float y, float z) {
    alSource3f(source, AL_POSITION, x, y, z);
  }

  void purrAudioEngine::setReverb(ALuint source, float gain) {
    alEffecti(mReverbEffect, AL_EFFECT_TYPE, AL_EFFECT_REVERB);
    alEffectf(mReverbEffect, AL_REVERB_GAIN, gain);
    alAuxiliaryEffectSloti(mEffectSlot, AL_EFFECTSLOT_EFFECT, mReverbEffect);
  }

  void purrAudioEngine::setPitch(ALuint source, float pitch) {
    alSourcef(source, AL_PITCH, pitch);
  }

  void purrAudioEngine::setGain(ALuint source, float gain) {
    alSourcef(source, AL_GAIN, gain);
  }
}