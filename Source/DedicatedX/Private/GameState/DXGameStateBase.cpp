// Fill out your copyright notice in the Description page of Project Settings.


#include "GameState/DXGameStateBase.h"

#include "Net/UnrealNetwork.h"

void ADXGameStateBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, AlivePlayerControllerCount);
	DOREPLIFETIME(ThisClass, MatchState);
}