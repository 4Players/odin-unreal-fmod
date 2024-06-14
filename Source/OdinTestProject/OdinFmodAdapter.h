// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FMODAudioComponent.h"
#include "fmod_studio.hpp"
#include "Components/ActorComponent.h"
#include "OdinFmodAdapter.generated.h"

class OdinMediaSoundGenerator;
class UOdinPlaybackMedia;

UENUM(BlueprintType)
enum class EFmodDspPan3dRolloffType : uint8 {
	FMOD_DSP_PAN_3D_ROLLOFF_LINEARSQUARED UMETA(DisplayName = "Linear Squared"),
	FMOD_DSP_PAN_3D_ROLLOFF_LINEAR UMETA(DisplayName = "Linear"),
	FMOD_DSP_PAN_3D_ROLLOFF_INVERSE UMETA(DisplayName = "Inverse"),
	FMOD_DSP_PAN_3D_ROLLOFF_INVERSETAPERED UMETA(DisplayName = "Inverse Tapered"),
	FMOD_DSP_PAN_3D_ROLLOFF_CUSTOM UMETA(DisplayName = "Custom")
};

UENUM(BlueprintType)
enum class EFmodDspPan3dExtentMode : uint8 {
	FMOD_DSP_PAN_3D_EXTENT_MODE_AUTO UMETA(DisplayName = "Auto"),
	FMOD_DSP_PAN_3D_EXTENT_MODE_USER UMETA(DisplayName = "User"),
	FMOD_DSP_PAN_3D_EXTENT_MODE_OFF UMETA(DisplayName = "Off")
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ODINTESTPROJECT_API UOdinFmodAdapter : public USceneComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	void BeginPlay() override;

	UOdinFmodAdapter();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Odin|Sound")
	void AssignOdinMedia(UPARAM(ref) UOdinPlaybackMedia*& Media);

	FMOD_RESULT dspreadcallback(FMOD_DSP_STATE* dsp_state, float* data, unsigned int datalen, int inchannels);

	// Object Spatializer Parameters

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EFmodDspPan3dRolloffType RolloffType = EFmodDspPan3dRolloffType::FMOD_DSP_PAN_3D_ROLLOFF_LINEARSQUARED;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0", UIMin = "0.0"))
	float MinimumDistance = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0", UIMin = "0.0"))
	float MaximumDistance = 2000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EFmodDspPan3dExtentMode ExtentMode = EFmodDspPan3dExtentMode::FMOD_DSP_PAN_3D_EXTENT_MODE_AUTO;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0", UIMin = "0.0"))
	float SoundSize = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "360.0", UIMin = "0.0", UIMax = "360.0"))
	float MinimumExtent = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (ClampMin = "0.0", ClampMax = "1.0", UIMin = "0.0", UIMax = "1.0"))
	float OutputGain = 0.0f;

	UFUNCTION(BlueprintCallable, Category = "Odin|Sound")
	void SetAttenuation(EFmodDspPan3dRolloffType InRolloffType, float InMinimumDistance, float InMaximumDistance, EFmodDspPan3dExtentMode InExtentMode, float InSoundSize, float InMinimumExtent, float InOutputGain);

	static FMOD_RESULT OdinDSPReadCallback(FMOD_DSP_STATE* dsp_state, float* inbuffer, float* outbuffer, unsigned int length, int inchannels, int* outchannels);

	FMOD::DSP* dsp_objectpan;

protected:

	UPROPERTY(BlueprintReadOnly, Category = "Odin|Sound")
	UOdinPlaybackMedia* PlaybackMedia = nullptr;
	TSharedPtr<OdinMediaSoundGenerator, ESPMode::ThreadSafe> SoundGenerator;

	void Update3DPosition();

	void UpdateAttenSettings();

	FMOD_VECTOR ConvertUnrealToFmodVector(FVector in);
};
