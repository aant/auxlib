#pragma once

#include "base.h"

namespace aux
{
	enum
	{
		SOUND_GROUP_BAD_ENUM = -1,

		SOUND_GROUP_AMBIENT,
		SOUND_GROUP_EFFECTS,
		SOUND_GROUP_SPEECH,

		SOUND_GROUP_MAX_ENUMS
	};

	struct sound_buffer_t;
	struct sound_t;

	struct AudioCaps
	{
		int32 maxBufferSize;
		int32 minSampleRate;
		int32 maxSampleRate;
	};

	void Audio_GetCaps(AudioCaps& caps);

	float32 get_sound_group_gain(enum32 group);
	void set_sound_group_gain(enum32 group, float32 gain);
	void suspend_sound_group(enum32 group);
	void resume_sound_group(enum32 group);

	int32 get_sound_buffer_size(const sound_buffer_t* buffer);
	sound_buffer_t* create_sound_buffer(int32 sample_rate, int32 sample_count, const void* data = nullptr);
	void destroy_sound_buffer(sound_buffer_t* buffer);
	void update_sound_buffer(sound_buffer_t* buffer, int32 offset, int32 size, const void* data);

	float32 get_sound_pos(const sound_t* sound);
	uint32 get_sound_len(const sound_t* sound);
	sound_t* create_sound(enum32 group);
	void destroy_sound(sound_t* sound);
	void set_sound_buffer(sound_t* sound, sound_buffer_t* buffer);
	void play_sound(sound_t* sound);
	void stop_sound(sound_t* sound);
	void loop_sound(sound_t* sound);
}
