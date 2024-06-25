#include "PurrfectEngine/PurrfectEngine.hpp"

#include <iostream>
#include <vector>
#include "minimp3_ex.h"
#include <sndfile.h>

namespace PurrfectEngine {

  purrAudioEngine::purrAudioEngine():
    mDevice(nullptr), mContext(nullptr), reverbEffect(0), effectSlot(0)
  {}

  purrAudioEngine::~purrAudioEngine() {
    cleanup();
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

    if (alcIsExtensionPresent(mDevice, "ALC_EXT_EFX")) {
      mEFXSupported = true;
      ALeffectslot(1, &mAuxEffectSlot);
    }

    return true;
  }

  void purrAudioEngine::cleanup() {
    if (effectSlot != 0) {
      alDeleteAuxiliaryEffectSlots(1, &effectSlot);
    }
    if (reverbEffect != 0) {
      alDeleteEffects(1, &reverbEffect);
    }
    alcMakeContextCurrent(nullptr);
    if (mContext != nullptr) {
      alcDestroyContext(mContext);
    }
    if (mDevice != nullptr) {
      alcCloseDevice(mDevice);
    }
  }

  bool purrAudioEngine::loadSound(const std::string &filename, ALuint &buffer) {
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

  bool purrAudioEngine::playSound(ALuint buffer) {
    ALuint source;
    alGenSources(1, &source);

    alSourcei(source, AL_BUFFER, buffer);
    alSourcef(source, AL_PITCH, 1);
    alSourcef(source, AL_GAIN, 1);
    alSource3f(source, AL_POSITION, 0, 0, 0);
    alSource3f(source, AL_VELOCITY, 0, 0, 0);
    alSourcei(source, AL_LOOPING, AL_FALSE);
    alSource3i(source, AL_AUXILIARY_SEND_FILTER, effectSlot, 0, AL_FILTER_NULL);

    alSourcePlay(source);

    ALint state;
    do {
      alGetSourcei(source, AL_SOURCE_STATE, &state);
    } while (state == AL_PLAYING);

    alDeleteSources(1, &source);
    return true;
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

  void purrAudioEngine::applyReverb(ALuint source, float gain) {
    alEffecti(reverbEffect, AL_EFFECT_TYPE, AL_EFFECT_REVERB);
    alEffectf(reverbEffect, AL_REVERB_GAIN, gain);
    alAuxiliaryEffect
  }
}