// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "DXGameStateBase.generated.h"

UENUM(BlueprintType)
enum class EMatchState : uint8
{
	None,
	Waiting,
	Playing,
	Ending,
	End
};

/**
 * 
 */
UCLASS()
class DEDICATEDX_API ADXGameStateBase : public AGameStateBase
{
	GENERATED_BODY()

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	int32 AlivePlayerControllerCount = 0;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly)
	EMatchState MatchState = EMatchState::Waiting;
};

