#include "audio.h"

#pragma warning(push, 0)

#define WIN32_LEAN_AND_MEAN
#define STRICT
#define INITGUID
#include <windows.h>
#include <mmsystem.h>
#include "windows__xaudio2.h"

#pragma warning(pop)

namespace aux
{
	enum
	{
		SUBMIX_VOICE_AMBIENT,
		SUBMIX_VOICE_EFFECTS,
		SUBMIX_VOICE_SPEECH,
		SUBMIX_VOICE_MUSIC,

		MAX_SUBMIX_VOICES
	};

	struct audio_t
	{
		HWND window;
		IXAudio2* device;
		IXAudio2MasteringVoice* master_voice;
		IXAudio2SubmixVoice* submix_voices[MAX_SUBMIX_VOICES];
	};

	static audio_t* audio = nullptr;

	///////////////////////////////////////////////////////////
	//
	//	Helper functions
	//
	///////////////////////////////////////////////////////////

	static void safe_release(IXAudio2Voice* voice)
	{
		if (voice != nullptr)
		{
			voice->DestroyVoice();
		}
	}

	static bool create_device_and_master_voice()
	{
		UINT32 flags = 0;

		#if defined(AUX_DEBUG_ON)
		flags |= XAUDIO2_DEBUG_ENGINE;
		#endif

		if (FAILED(XAudio2Create(&(audio->device), flags, XAUDIO2_DEFAULT_PROCESSOR)))
		{
			return false;
		}

		if (FAILED(audio->device->CreateMasteringVoice(&(audio->master_voice), XAUDIO2_DEFAULT_CHANNELS, XAUDIO2_DEFAULT_SAMPLERATE, 0, 0, nullptr)))
		{
			return false;
		}

		return true;
	}

	static bool create_submix_voice(IXAudio2SubmixVoice** voice, UINT32 sample_rate, UINT32 processing_stage)
	{
		return SUCCEEDED(audio->device->CreateSubmixVoice(voice, 2, sample_rate, 0, processing_stage, nullptr, nullptr));
	}

	///////////////////////////////////////////////////////////
	//
	//	Internal functions
	//
	///////////////////////////////////////////////////////////

	bool internal__init_audio(HWND window)
	{
		audio = (audio_t*)zalloc_mem(sizeof(audio_t));
		audio->window = window;

		if (!create_device_and_master_voice())
		{
			return false;
		}

		for (UINT32 i = 0; i < MAX_SUBMIX_VOICES; ++i)
		{
			if (!create_submix_voice(audio->submix_voices + i, 44100, i))
			{
				return false;
			}
		}

		return true;
	}

	void internal__free_audio()
	{
		if (audio != nullptr)
		{
			if (audio->device != nullptr)
			{
				audio->device->StartEngine();

				for (UINT32 i = 0; i < MAX_SUBMIX_VOICES; ++i)
				{
					safe_release(audio->submix_voices[(UINT32)MAX_SUBMIX_VOICES - i - 1]);
				}

				safe_release(audio->master_voice);
				audio->device->Release();
			}

			free_mem(audio);
			audio = nullptr;
		}
	}

	void internal__suspend_audio_playback()
	{
		audio->device->StopEngine();
	}

	void internal__resume_audio_playback()
	{
		audio->device->StartEngine();
	}
}
