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
	#pragma pack(1)

	struct audio_t
	{
		HWND window;
		IXAudio2* device;
		IXAudio2MasteringVoice* master_voice;
		IXAudio2SubmixVoice* submix_voice_ambient;
		IXAudio2SubmixVoice* submix_voice_effects;
		IXAudio2SubmixVoice* submix_voice_speech;
		IXAudio2SubmixVoice* submix_voice_music;
		audio_caps_t caps;
	};

	#pragma pack()

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

	static void handle_critical_error(const wchar_t message[], HRESULT result)
	{
		MessageBoxW(GetForegroundWindow(), message, L"CRITICAL ERROR", MB_TOPMOST | MB_TASKMODAL | MB_ICONERROR | MB_OK);
		ExitProcess((UINT)-1);
		(void)result;
	}

	static void check_for_critical_error(HRESULT result)
	{
		switch (result)
		{
			case XAUDIO2_E_DEVICE_INVALIDATED:
				handle_critical_error(L"Audio device lost.", result);
				break;
		}
	}

	static bool create_device_and_master_voice()
	{
		UINT32 flags = 0;

		#if defined(AUX_DEBUG_ON)
		flags |= XAUDIO2_DEBUG_ENGINE;
		#endif

		HRESULT result = XAudio2Create(&(audio->device), flags, XAUDIO2_DEFAULT_PROCESSOR);

		if (FAILED(result))
		{
			check_for_critical_error(result);
			return false;
		}

		result = audio->device->CreateMasteringVoice(&(audio->master_voice), XAUDIO2_DEFAULT_CHANNELS, XAUDIO2_DEFAULT_SAMPLERATE, 0, 0, nullptr);

		if (FAILED(result))
		{
			check_for_critical_error(result);
			return false;
		}

		return true;
	}

	static bool create_submix_voice(IXAudio2SubmixVoice** voice, UINT32 sample_rate, UINT32 processing_stage)
	{
		HRESULT result = audio->device->CreateSubmixVoice(voice, 2, sample_rate, 0, processing_stage, nullptr, nullptr);

		if (SUCCEEDED(result))
		{
			return true;
		}

		check_for_critical_error(result);
		return false;
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

		UINT32 rate = 44100;
		UINT32 stage = 0;

		if (!create_submix_voice(&(audio->submix_voice_ambient), rate, stage++))
		{
			return false;
		}

		if (!create_submix_voice(&(audio->submix_voice_effects), rate, stage++))
		{
			return false;
		}

		if (!create_submix_voice(&(audio->submix_voice_speech), rate, stage++))
		{
			return false;
		}

		if (!create_submix_voice(&(audio->submix_voice_music), rate, stage++))
		{
			return false;
		}

		audio->caps.max_buffer_size = XAUDIO2_MAX_BUFFER_BYTES;
		audio->caps.min_sample_rate = XAUDIO2_MIN_SAMPLE_RATE;
		audio->caps.max_sample_rate = XAUDIO2_MAX_SAMPLE_RATE;
		return true;
	}

	void internal__free_audio()
	{
		if (audio != nullptr)
		{
			if (audio->device != nullptr)
			{
				audio->device->StartEngine();
				safe_release(audio->submix_voice_music);
				safe_release(audio->submix_voice_speech);
				safe_release(audio->submix_voice_effects);
				safe_release(audio->submix_voice_ambient);
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
