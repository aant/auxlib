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

	struct audio_caps_t
	{
		i32_t max_buffer_size;
		i32_t min_sample_rate;
		i32_t max_sample_rate;
	};

	void get_audio_caps(audio_caps_t& caps);

	f32_t get_sound_group_gain(e32_t group);
	void set_sound_group_gain(e32_t group, f32_t gain);
	void suspend_sound_group(e32_t group);
	void resume_sound_group(e32_t group);

	i32_t get_sound_buffer_size(const sound_buffer_t* buffer);
	sound_buffer_t* create_sound_buffer(i32_t sample_rate, i32_t sample_count, const void* data = nullptr);
	void destroy_sound_buffer(sound_buffer_t* buffer);
	void update_sound_buffer(sound_buffer_t* buffer, i32_t offset, i32_t size, const void* data);

	f32_t get_sound_pos(const sound_t* sound);
	u32_t get_sound_len(const sound_t* sound);
	sound_t* create_sound(e32_t group);
	void destroy_sound(sound_t* sound);
	void set_sound_buffer(sound_t* sound, sound_buffer_t* buffer);
	void play_sound(sound_t* sound);
	void stop_sound(sound_t* sound);
	void loop_sound(sound_t* sound);
}
