// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "DXGameModeBase.generated.h"

class ADXPlayerController;

/**
 * 
 */
UCLASS()
class DEDICATEDX_API ADXGameModeBase : public AGameModeBase
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

	virtual void PostLogin(APlayerController* NewPlayer) override;

	virtual void Logout(AController* Exiting) override;

	void OnCharacterDead(ADXPlayerController* InController);

private:
	UFUNCTION()
	void OnMainTimerElapsed();

	void NotifyToAllPlayer(const FString& NotificationString);

public:
	FTimerHandle MainTimerHandle;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 WaitingTime = 15;

	int32 RemainWaitingTimeForPlaying = 15;

	int32 MinimumPlayerCountForPlaying = 2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 EndingTime = 15;

	int32 RemainWaitingTimeForEnding = 15;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<TObjectPtr<ADXPlayerController>> AlivePlayerControllers;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<TObjectPtr<ADXPlayerController>> DeadPlayerControllers;

};