// Fill out your copyright notice in the Description page of Project Settings.

#include "fmod_studio.hpp"
#include "FMODStudioModule.h"
#include "odin.h"
#include "OdinFunctionLibrary.h"
#include "OdinMediaSoundGenerator.h"
#include "OdinPlaybackMedia.h"
#include "OdinFmodAdapter.h"

static FMOD_RESULT F_CALLBACK onodinpcmreadcallback(FMOD_SOUND* inSound, void* data, unsigned int datalen)
{
	FMOD::Sound* sound = (FMOD::Sound*)inSound;
	void* userdata;

	sound->getUserData(&userdata);

	UOdinFmodAdapter* instance = reinterpret_cast<UOdinFmodAdapter*>(userdata);

	return instance->pcmreadcallback(inSound, data, datalen);
}

void UOdinFmodAdapter::AssignOdinMedia(UOdinPlaybackMedia*& Media)
{
	if (nullptr == Media)
		return;

	this->SoundGenerator = MakeShared<OdinMediaSoundGenerator, ESPMode::ThreadSafe>();
	this->PlaybackMedia = Media;

	SoundGenerator->SetOdinStream(Media->GetMediaHandle());
}

UOdinFmodAdapter::UOdinFmodAdapter()
{
	PrimaryComponentTick.bCanEverTick = true;

	FMOD::Studio::System* System = IFMODStudioModule::Get().GetStudioSystem(EFMODSystemContext::Runtime);
	FMOD::System* CoreSystem = nullptr;
	System->getCoreSystem(&CoreSystem);

	FMOD_CREATESOUNDEXINFO SoundInfo = { 0 };
	SoundInfo.cbsize = sizeof(FMOD_CREATESOUNDEXINFO);
	SoundInfo.format = FMOD_SOUND_FORMAT_PCMFLOAT;
	SoundInfo.defaultfrequency = 48000;
	SoundInfo.numchannels = 2;
	SoundInfo.pcmreadcallback = onodinpcmreadcallback;
	SoundInfo.length = (unsigned int)(48000 * sizeof(float) * 2);
	SoundInfo.userdata = this;

	if (CoreSystem->createStream("", FMOD_OPENUSER | FMOD_LOOP_NORMAL, &SoundInfo, &Sound) == FMOD_OK)
	{
		FMOD::ChannelGroup* group;
		CoreSystem->getMasterChannelGroup(&group);
		FMOD::Channel* channel;
		CoreSystem->playSound(Sound, group, false, &channel);
	}
}

FMOD_RESULT UOdinFmodAdapter::pcmreadcallback(FMOD_SOUND* inSound, void* data, unsigned int datalen)
{
	if (!data)
		return FMOD_ERR_INVALID_PARAM;

	if (!SoundGenerator || !PlaybackMedia)
		return FMOD_OK;

	unsigned int requestedDataArrayLength = datalen / sizeof(float);

	if (this->BufferSize < requestedDataArrayLength)
	{
		if (nullptr != Buffer)
			delete Buffer;
		Buffer = new float[requestedDataArrayLength];

		BufferSize = requestedDataArrayLength;
	}

	float* fltData = reinterpret_cast<float*>(data);

	const uint32 Result = SoundGenerator->OnGenerateAudio(Buffer, (int32)requestedDataArrayLength);

	if (odin_is_error(Result))
	{
		FString ErrorString = UOdinFunctionLibrary::FormatError(Result, true);
		UE_LOG(LogTemp, Error, TEXT("UOdinFmodAdapter: Error during FillSamplesBuffer: %s"), *ErrorString);
		return FMOD_OK;
	}

	memcpy(data, Buffer, datalen);

	return FMOD_OK;
}

void UOdinFmodAdapter::DestroyComponent(bool bPromoteChildren)
{
	Super::DestroyComponent(bPromoteChildren);
	if (nullptr != Buffer)
	{
		delete Buffer;
		Buffer = nullptr;
		BufferSize = 0;
	}
}

