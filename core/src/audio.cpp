#include "PurrfectEngine/PurrfectEngine.hpp"

#include <iostream>
#include <vector>
#include "minimp3_ex.h"
#include <sndfile.h>

namespace PurrfectEngine
{
    AudioEngine::AudioEngine() : device(nullptr), context(nullptr) {}
    AudioEngine::~AudioEngine()
    {
        alcMakeContextCurrent(nullptr);
        alcDestroyContext(context);
        alcCloseDevice(device);
    }

    void AudioEngine::init()
    {
        device = alcOpenDevice(nullptr);
        if (!device)
        {
            std::cerr << "Failed to open sound device" << std::endl;
            return;
        }

        context = alcCreateContext(device, nullptr);
        if (!alcMakeContextCurrent(context))
        {
            std::cerr << "Failed to set sound context" << std::endl;
            return;
        }

        alListener3f(AL_POSITION, 0.0f, 0.0f, 0.0f);
        alListener3f(AL_VELOCITY, 0.0f, 0.0f, 0.0f);
        ALfloat listenerOri[] = {0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f};
        alListenerfv(AL_ORIENTATION, listenerOri);
    };

    void AudioEngine::loadSound(const std::string &filename, ALuint &buffer)
    {
        std::string extension = filename.substr(filename.find_last_of(".") + 1);

        if (extension == "mp3")
        {
            mp3dec_t mp3d;
            mp3dec_file_info_t info;
            if (mp3dec_load(&mp3d, filename.c_str(), &info, nullptr, nullptr))
            {
                std::cerr << "Failed to load MP3 file: " << filename << std::endl;
                return;
            }
            ALenum format = info.channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
            alGenBuffers(1, &buffer);
            alBufferData(buffer, format, info.buffer, info.samples * sizeof(mp3d_sample_t), info.hz);

            free(info.buffer);
        }
        else
        {
            SF_INFO sfInfo;
            SNDFILE *sndFile = sf_open(filename.c_str(), SFM_READ, &sfInfo);
            if (!sndFile)
            {
                std::cerr << "Failed to load sound file: " << filename << std::endl;
                return;
            }

            std::vector<short> samples(sfInfo.frames * sfInfo.channels);
            sf_read_short(sndFile, samples.data(), sfInfo.frames * sfInfo.channels);
            sf_close(sndFile);

            ALenum format;
            if (sfInfo.channels == 1)
            {
                format = AL_FORMAT_MONO16;
            }
            else if (sfInfo.channels == 2)
            {
                format = AL_FORMAT_STEREO16;
            }
            else
            {
                std::cerr << "Unsupported channel count: " << sfInfo.channels << std::endl;
                return;
            }

            alGenBuffers(1, &buffer);
            alBufferData(buffer, format, samples.data(), samples.size() * sizeof(short), sfInfo.samplerate);
        }
    };

    void AudioEngine::playSound(ALuint buffer)
    {
        ALuint source;
        alGenSources(1, &source);

        alSourcei(source, AL_BUFFER, buffer);
        alSourcef(source, AL_PITCH, 1);
        alSourcef(source, AL_GAIN, 1);
        alSource3f(source, AL_POSITION, 0, 0, 0);
        alSource3f(source, AL_VELOCITY, 0, 0, 0);
        alSourcei(source, AL_LOOPING, AL_FALSE);

        alSourcePlay(source);

        ALint state;
        do
        {
            alGetSourcei(source, AL_SOURCE_STATE, &state);
        } while (state == AL_PLAYING);

        alDeleteSources(1, &source);
    };
}
