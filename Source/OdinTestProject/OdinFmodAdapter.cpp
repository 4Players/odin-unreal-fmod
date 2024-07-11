// Fill out your copyright notice in the Description page of Project Settings.

#include "OdinFmodAdapter.h"
#include "fmod_studio.hpp"
#include "FMODStudioModule.h"
#include "odin.h"
#include "OdinFunctionLibrary.h"
#include "OdinMediaSoundGenerator.h"
#include "OdinPlaybackMedia.h"
#include <Kismet/KismetMathLibrary.h>

void UOdinFmodAdapter::AssignOdinMedia(UOdinPlaybackMedia*& Media)
{
	if (nullptr == Media)
		return;

	this->SoundGenerator = MakeShared<OdinMediaSoundGenerator, ESPMode::ThreadSafe>();
	this->PlaybackMedia = Media;

	SoundGenerator->SetOdinStream(Media->GetMediaHandle());
}

void UOdinFmodAdapter::SetAttenuation(EFmodDspPan3dRolloffType InRolloffType, float InMinimumDistance, float InMaximumDistance, EFmodDspPan3dExtentMode InExtentMode, float InSoundSize, float InMinimumExtent, float InOutputGain)
{
	this->RolloffType = InRolloffType;
	this->MinimumDistance = InMinimumDistance;
	this->MaximumDistance = InMaximumDistance;
	this->ExtentMode = InExtentMode;
	this->SoundSize = InSoundSize;
	this->MinimumExtent = InMinimumExtent;
	this->OutputGain = InOutputGain;

	UpdateAttenSettings();

	Update3DPosition();
}

FMOD_RESULT UOdinFmodAdapter::OdinDSPReadCallback(FMOD_DSP_STATE* dsp_state, float* inbuffer, float* outbuffer, unsigned int length, int inchannels, int* outchannels)
{
	void* userdata;

	dsp_state->functions->getuserdata(dsp_state, &userdata);

	*outchannels = 2;

	UOdinFmodAdapter* instance = reinterpret_cast<UOdinFmodAdapter*>(userdata);

	return instance->dspreadcallback(dsp_state, outbuffer, length, inchannels);
}

void UOdinFmodAdapter::Update3DPosition()
{
	FMOD_DSP_PARAMETER_3DATTRIBUTES_MULTI attrs = { 0 };

	FMOD_3D_ATTRIBUTES relattr = { 0 };

	FMOD_3D_ATTRIBUTES absattr = { 0 };

	if (GEngine == nullptr) return;

	UWorld* world = this->GetWorld();
	if (world == nullptr) return;

	ULocalPlayer* player = GEngine->GetGamePlayer(world, 0);
	if (player == nullptr) return;

	TObjectPtr<APlayerController> controller = player->PlayerController;
	if (controller == nullptr) return;

	AActor* listener = controller->GetPawn();
	if (listener == nullptr) return;

	relattr.position = ConvertUnrealToFmodVector(this->GetComponentLocation() - listener->GetActorLocation(), 0.01f);
	relattr.velocity = ConvertUnrealToFmodVector(this->GetComponentVelocity() - listener->GetVelocity(), 0.01f);
	relattr.forward = ConvertUnrealToFmodVector(UKismetMathLibrary::GetForwardVector(this->GetComponentRotation()));
	relattr.up = ConvertUnrealToFmodVector(UKismetMathLibrary::GetUpVector(this->GetComponentRotation()));

	absattr.position = ConvertUnrealToFmodVector(this->GetComponentLocation(), 0.01f);
	absattr.velocity = ConvertUnrealToFmodVector(this->GetComponentVelocity(), 0.01f);
	absattr.forward = ConvertUnrealToFmodVector(UKismetMathLibrary::GetForwardVector(this->GetComponentRotation()));
	absattr.up = ConvertUnrealToFmodVector(UKismetMathLibrary::GetUpVector(this->GetComponentRotation()));

	attrs.relative[0] = relattr;
	attrs.numlisteners = 1;
	attrs.absolute = absattr;

	dsp_pan->setParameterData(FMOD_DSP_PAN_3D_POSITION, &attrs, sizeof(FMOD_DSP_PARAMETER_3DATTRIBUTES_MULTI));

	group->set3DAttributes(&absattr.position, &absattr.velocity);

	FMOD::Studio::System* System = IFMODStudioModule::Get().GetStudioSystem(EFMODSystemContext::Runtime);
	FMOD::System* CoreSystem = nullptr;
	System->getCoreSystem(&CoreSystem);

	FMOD_VECTOR pos = ConvertUnrealToFmodVector(listener->GetActorLocation(), 0.01f);
	FMOD_VECTOR vel = ConvertUnrealToFmodVector(listener->GetVelocity(), 0.01f);
	FMOD_VECTOR forward = ConvertUnrealToFmodVector(listener->GetActorForwardVector());
	FMOD_VECTOR up = ConvertUnrealToFmodVector(listener->GetActorUpVector());

	CoreSystem->set3DListenerAttributes(0, &pos, &vel, &forward, &up);
	CoreSystem->update();
}

void UOdinFmodAdapter::UpdateAttenSettings()
{
	dsp_pan->setParameterInt(FMOD_DSP_PAN_3D_ROLLOFF, (int)RolloffType);
	dsp_pan->setParameterFloat(FMOD_DSP_PAN_3D_MIN_DISTANCE, MinimumDistance);
	dsp_pan->setParameterFloat(FMOD_DSP_PAN_3D_MAX_DISTANCE, MaximumDistance);
	dsp_pan->setParameterInt(FMOD_DSP_PAN_3D_EXTENT_MODE, (int)ExtentMode);
	dsp_pan->setParameterFloat(FMOD_DSP_PAN_3D_SOUND_SIZE, SoundSize);
	dsp_pan->setParameterFloat(FMOD_DSP_PAN_3D_MIN_EXTENT, MinimumExtent);
}

FMOD_VECTOR UOdinFmodAdapter::ConvertUnrealToFmodVector(FVector in, float scale)
{
	auto out = FMOD_VECTOR();
	out.x = in.Y * scale;
	out.y = in.Z * scale;
	out.z = in.X * scale;

	return out;
}

void UOdinFmodAdapter::BeginPlay()
{
	FMOD::Studio::System* System = IFMODStudioModule::Get().GetStudioSystem(EFMODSystemContext::Runtime);
	FMOD::System* CoreSystem = nullptr;
	System->getCoreSystem(&CoreSystem);

	FMOD_DSP_READ_CALLBACK mReadCallback = OdinDSPReadCallback;

	FMOD_DSP_DESCRIPTION desc = { 0 };
	desc.read = mReadCallback;
	desc.userdata = this;
	desc.numoutputbuffers = 1;

	FMOD_RESULT res = CoreSystem->createDSP(&desc, &mOdinDSP);

	if (res == FMOD_RESULT::FMOD_OK)
	{
		CoreSystem->getMasterChannelGroup(&group);

		if (group->addDSP(0, mOdinDSP) == FMOD_RESULT::FMOD_OK)
		{
			UE_LOG(LogTemp, Warning, TEXT("Added Odin DSP to channel group"));
		}
	}

	FMOD_RESULT result = CoreSystem->createDSPByType(FMOD_DSP_TYPE_PAN, &dsp_pan);

	

	if (result == FMOD_RESULT::FMOD_OK)
	{
		CoreSystem->getMasterChannelGroup(&group);

		if (group->addDSP(FMOD_CHANNELCONTROL_DSP_HEAD, dsp_pan) == FMOD_RESULT::FMOD_OK)
		{
			UE_LOG(LogTemp, Warning, TEXT("Added Object Spatializer DSP to channel group"));

			group->setMode(FMOD_3D | FMOD_3D_WORLDRELATIVE | FMOD_3D_LINEARROLLOFF);
		}
	}

	// Set Pan Output to Stereo
	dsp_pan->setParameterInt(FMOD_DSP_PAN_MODE, (int)FMOD_DSP_PAN_MODE_SURROUND);

	// Set Pan Mode to full 3D Positional
	dsp_pan->setParameterFloat(FMOD_DSP_PAN_3D_PAN_BLEND, 1.0f);

	UpdateAttenSettings();
	Update3DPosition();
}

void UOdinFmodAdapter::DestroyComponent(bool bPromoteChildren)
{
	auto result2 = group->removeDSP(dsp_pan);
	auto result4 = group->removeDSP(mOdinDSP);

	dsp_pan->release();
	mOdinDSP->release();

	Super::DestroyComponent(bPromoteChildren);
}

UOdinFmodAdapter::UOdinFmodAdapter()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UOdinFmodAdapter::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	Update3DPosition();
}

FMOD_RESULT UOdinFmodAdapter::dspreadcallback(FMOD_DSP_STATE* dsp_state, float* data, unsigned int datalen, int inchannels)
{
	if (!data)
		return FMOD_ERR_INVALID_PARAM;

	if (!SoundGenerator || !PlaybackMedia)
		return FMOD_OK;

	unsigned int requestedDataArrayLength = datalen * 2;

	const uint32 Result = SoundGenerator->OnGenerateAudio(data, (int32)requestedDataArrayLength);

	if (odin_is_error(Result))
	{
		FString ErrorString = UOdinFunctionLibrary::FormatError(Result, true);
		UE_LOG(LogTemp, Error, TEXT("UOdinFmodAdapter: Error during FillSamplesBuffer: %s"), *ErrorString);
		return FMOD_OK;
	}

	return FMOD_OK;
}

