#include "audio.h"
#include "stb_vorbis.h"
#include "util.h"
#include "platform.h"

const char * GetOpenALErrorString(int errID) {
	if (errID == AL_NO_ERROR) return "";
	if (errID == AL_INVALID_NAME) return "Invalid name";
    if (errID == AL_INVALID_ENUM) return " Invalid enum ";
    if (errID == AL_INVALID_VALUE) return " Invalid value ";
    if (errID == AL_INVALID_OPERATION) return " Invalid operation ";
    if (errID == AL_OUT_OF_MEMORY) return " Out of memory like! ";
    return " Don't know ";
}

void CheckOpenALError() {
	ALenum err = alGetError();
    if (err != AL_NO_ERROR) {
		uerr(GetOpenALErrorString(err));
    }
}

#ifdef DEBUG
   #define AL_CHECK(stmt) do { \
        stmt; \
        CheckOpenALError(); \
    } while (0);
#else
    #define AL_CHECK(stmt) stmt
#endif

static ALCdevice * audio_AL_device;
static ALCcontext * audio_AL_context;

void audio_init() {

    alGetError();

    audio_AL_device = alcOpenDevice(NULL);

    if (audio_AL_device == NULL) {
        uerr("Failed to init OpenAL device.");
        return;
    }

    audio_AL_context  = alcCreateContext(audio_AL_device, NULL);
    AL_CHECK( alcMakeContextCurrent(audio_AL_context) );

}

void audio_deinit() {
    alcMakeContextCurrent(NULL);
    alcDestroyContext(audio_AL_context);
    alcCloseDevice(audio_AL_device);
}

SoundData * audio_data_init(SoundData * data, const char * resource) {

    int channels;
	int sample_rate;
	short * s_data;
	int samples = stb_vorbis_decode_filename(platform_res2file_ez(resource), &channels, &sample_rate, &s_data);
    ALuint buffer;
    alGenBuffers(1, &buffer);
	ALenum format = channels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
    alBufferData(buffer, format, s_data, samples * channels * sizeof(short), sample_rate);
    free(s_data);

    data->samples = samples;
    data->buffer = buffer;
    data->channels = channels;
    data->sample_rate = sample_rate;
    data->resource = resource;

    return data;
}

void audio_data_deinit(SoundData * data) {
    alDeleteBuffers(1, &data->buffer);
}

Sound * audio_sound_init(Sound * sound, SoundData * data) {
    static const vec3 zero = {0, 0, 0};
    vec3_assign(sound->position, zero);
    sound->data = data;
    sound->flags = AUDIO_ACTIVE;
    sound->volume = 1;
    sound->pitch = 1;
    sound->gain = 1;
    alGenSources(1, &sound->source);
    AL_CHECK(alSourceQueueBuffers(sound->source, 1, &data->buffer));
    audio_sound_update(sound);
    return sound;
}

Sound * audio_sound_init_resource(Sound * sound, const char * resource) {
    SoundData * data = malloc(sizeof(SoundData));
    audio_data_init(data, resource);
    audio_sound_init(sound, data);
    sound->flags |= AUDIO_OWNS_DATA;
    return sound;
}

void audio_sound_deinit(Sound * sound) {
    if (sound->flags & AUDIO_ACTIVE) {
        sound->flags &= ~AUDIO_ACTIVE;
        alDeleteSources(1, &sound->source);
        if (sound->flags & AUDIO_OWNS_DATA) {
            audio_data_deinit(sound->data);
            free(sound->data);
        }
    }
}

void audio_sound_update(Sound * sound) {
    alSourcef(sound->source, AL_PITCH, sound->pitch);
    alSourcef(sound->source, AL_GAIN, sound->gain);
    alSource3f(sound->source, AL_POSITION,
            sound->position[0],
            sound->position[1],
            sound->position[2]);
}

void audio_sound_play(Sound * sound) {
    alSourcei(sound->source, AL_LOOPING, AL_FALSE);
    alSourcePlay(sound->source);
}

void audio_sound_persist(Sound * sound) {
    ALint source_state;
    AL_CHECK(alGetSourcei(sound->source, AL_SOURCE_STATE, &source_state));
    if (source_state == AL_STOPPED || source_state == AL_INITIAL) {
        audio_sound_play(sound);
    }
}

void audio_sound_stop(Sound * sound) {
    alSourceStop(sound->source);
}

void audio_sound_stop_looping(Sound * sound) {
    alSourcei(sound->source, AL_LOOPING, AL_FALSE);
}

void audio_sound_loop(Sound * sound) {
    alSourcei(sound->source, AL_LOOPING, AL_TRUE);
    alSourcePlay(sound->source);
}
