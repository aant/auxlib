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
		SubmixVoice_Ambient,
		SubmixVoice_Effects,
		SubmixVoice_Speech,
		SubmixVoice_Music,

		SubmixVoice_MaxEnums
	};

	struct Audio
	{
		HWND window;
		IXAudio2* device;
		IXAudio2MasteringVoice* masterVoice;
		IXAudio2SubmixVoice* submixVoices[SubmixVoice_MaxEnums];
	};

	static Audio* audio = nullptr;

	///////////////////////////////////////////////////////////
	//
	//	Helper functions
	//
	///////////////////////////////////////////////////////////

	static void LL_SafeRelease(IXAudio2Voice* voice)
	{
		if (voice != nullptr)
		{
			voice->DestroyVoice();
		}
	}

	static bool LL_CreateDeviceAndMasterVoice()
	{
		UINT32 flags = 0;

		#if defined(AUX_DEBUG_ON)
		flags |= XAUDIO2_DEBUG_ENGINE;
		#endif

		if (FAILED(XAudio2Create(&(audio->device), flags, XAUDIO2_DEFAULT_PROCESSOR)))
		{
			return false;
		}

		if (FAILED(audio->device->CreateMasteringVoice(&(audio->masterVoice), XAUDIO2_DEFAULT_CHANNELS, XAUDIO2_DEFAULT_SAMPLERATE, 0, 0, nullptr)))
		{
			return false;
		}

		return true;
	}

	static bool LL_CreateSubmixVoice(IXAudio2SubmixVoice** voice, UINT32 sample_rate, UINT32 processing_stage)
	{
		return SUCCEEDED(audio->device->CreateSubmixVoice(voice, 2, sample_rate, 0, processing_stage, nullptr, nullptr));
	}

	///////////////////////////////////////////////////////////
	//
	//	Internal functions
	//
	///////////////////////////////////////////////////////////

	bool Internal__InitAudio(HWND window)
	{
		audio = (Audio*)Memory_AllocateAndZero(sizeof(Audio));
		audio->window = window;

		if (!LL_CreateDeviceAndMasterVoice())
		{
			return false;
		}

		for (UINT32 i = 0; i < SubmixVoice_MaxEnums; ++i)
		{
			if (!LL_CreateSubmixVoice(audio->submixVoices + i, 44100, i))
			{
				return false;
			}
		}

		return true;
	}

	void Internal__FreeAudio()
	{
		if (audio != nullptr)
		{
			if (audio->device != nullptr)
			{
				audio->device->StartEngine();

				for (UINT32 i = 0; i < SubmixVoice_MaxEnums; ++i)
				{
					LL_SafeRelease(audio->submixVoices[(UINT32)SubmixVoice_MaxEnums - i - 1]);
				}

				LL_SafeRelease(audio->masterVoice);
				audio->device->Release();
			}

			Memory_Free(audio);
			audio = nullptr;
		}
	}

	void Internal__SuspendAudioPlayback()
	{
		audio->device->StopEngine();
	}

	void Internal__ResumeAudioPlayback()
	{
		audio->device->StartEngine();
	}
}
