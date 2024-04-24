// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FMODAudioComponent.h"
#include "fmod_studio.hpp"
#include "Components/ActorComponent.h"
#include "OdinFmodAdapter.generated.h"

class OdinMediaSoundGenerator;
class UOdinPlaybackMedia;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ODINTESTPROJECT_API UOdinFmodAdapter : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	UOdinFmodAdapter();

	UFUNCTION(BlueprintCallable, Category = "Odin|Sound")
	void AssignOdinMedia(UPARAM(ref) UOdinPlaybackMedia*& Media);

	FMOD_RESULT pcmreadcallback(FMOD_SOUND* inSound, void* data, unsigned int datalen);

	void DestroyComponent(bool bPromoteChildren) override;

protected:

	UPROPERTY(BlueprintReadOnly, Category = "Odin|Sound")
	UOdinPlaybackMedia* PlaybackMedia = nullptr;
	TSharedPtr<OdinMediaSoundGenerator, ESPMode::ThreadSafe> SoundGenerator;

	float* Buffer = nullptr;
	unsigned int BufferSize = 0;


	FMOD::Sound* Sound = nullptr;
};
