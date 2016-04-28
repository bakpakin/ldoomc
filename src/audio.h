#ifndef AUDIO_H_WMDLVZMG
#define AUDIO_H_WMDLVZMG

#include <stdint.h>
#include <AL/al.h>
#include <AL/alc.h>
#include "stb_vorbis.h"
#include "ldmath.h"

void audio_init();

void audio_deinit();

#define AUDIO_OWNS_DATA 0x04

typedef struct {
    ALuint buffer;
    const char * resource;
    int samples;
    int channels;
    int sample_rate;
} SoundData;

typedef struct {
    uint32_t flags;
    SoundData * data;
    ALuint source;
    vec3 position;
    float volume;
    float pitch;
    float gain;
} Sound;

SoundData * audio_data_init(SoundData * data, const char * resource);

void audio_data_deinit(SoundData * data);

Sound * audio_sound_init(Sound * sound, SoundData * data);

Sound * audio_sound_init_resource(Sound * sound, const char * resource);

void audio_sound_deinit(Sound * sound);

void audio_sound_play(Sound * sound);

void audio_sound_persist(Sound * sound);

void audio_sound_update(Sound * sound);

void audio_sound_stop(Sound * sound);

void audio_sound_stop_looping(Sound * sound);

void audio_sound_loop(Sound * sound);

#endif /* end of include guard: AUDIO_H_WMDLVZMG */
