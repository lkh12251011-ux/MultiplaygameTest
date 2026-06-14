// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/DXGameModeBase.h"

#include "GameState/DXGameStateBase.h"
#include "Player/DXPlayerController.h"
#include "Kismet/GameplayStatics.h"

void ADXGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	GetWorld()->GetTimerManager().SetTimer(MainTimerHandle, this, &ThisClass::OnMainTimerElapsed, 1.f, true);

	RemainWaitingTimeForPlaying = WaitingTime;

	RemainWaitingTimeForEnding = EndingTime;
}

void ADXGameModeBase::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	ADXGameStateBase* DXGameState = GetGameState<ADXGameStateBase>();
	if (IsValid(DXGameState) == false)
	{
		return;
	}

	if (DXGameState->MatchState != EMatchState::Waiting)
	{
		NewPlayer->SetLifeSpan(0.1f);
		return;
	}

	ADXPlayerController* NewPlayerController = Cast<ADXPlayerController>(NewPlayer);
	if (IsValid(NewPlayerController) == true)
	{
		AlivePlayerControllers.Add(NewPlayerController);

		NewPlayerController->NotificationText = FText::FromString(TEXT("Connected to the game server."));
	}
}

void ADXGameModeBase::Logout(AController* Exiting)
{
	Super::Logout(Exiting);

	ADXPlayerController* ExitingPlayerController = Cast<ADXPlayerController>(Exiting);
	if (IsValid(ExitingPlayerController) == true && AlivePlayerControllers.Find(ExitingPlayerController) != INDEX_NONE)
	{
		AlivePlayerControllers.Remove(ExitingPlayerController);
		DeadPlayerControllers.Add(ExitingPlayerController);
	}
}

void ADXGameModeBase::NotifyToAllPlayer(const FString& NotificationString)
{
	for (auto AlivePlayerController : AlivePlayerControllers)
	{
		AlivePlayerController->NotificationText = FText::FromString(NotificationString);
	}

	for (auto DeadPlayerController : DeadPlayerControllers)
	{
		DeadPlayerController->NotificationText = FText::FromString(NotificationString);
	}
}

void ADXGameModeBase::OnMainTimerElapsed()
{
	ADXGameStateBase* DXGameState = GetGameState<ADXGameStateBase>();
	if (IsValid(DXGameState) == false)
	{
		return;
	}

	switch (DXGameState->MatchState)
	{
	case EMatchState::None:
		break;
	case EMatchState::Waiting:
	{
		FString NotificationString = FString::Printf(TEXT(""));

		if (AlivePlayerControllers.Num() < MinimumPlayerCountForPlaying)
		{
			NotificationString = FString::Printf(TEXT("Wait another players for playing."));

			RemainWaitingTimeForPlaying = WaitingTime;
		}
		else
		{
			NotificationString = FString::Printf(TEXT("Wait %d seconds for playing."), RemainWaitingTimeForPlaying);

			--RemainWaitingTimeForPlaying;
		}

		if (RemainWaitingTimeForPlaying <= 0)
		{
			NotificationString = FString::Printf(TEXT(""));

			DXGameState->MatchState = EMatchState::Playing;
		}

		NotifyToAllPlayer(NotificationString);

		break;
	}
	case EMatchState::Playing:
	{
		DXGameState->AlivePlayerControllerCount = AlivePlayerControllers.Num();

		FString NotificationString = FString::Printf(TEXT("%d / %d"), DXGameState->AlivePlayerControllerCount, DXGameState->AlivePlayerControllerCount + DeadPlayerControllers.Num());

		NotifyToAllPlayer(NotificationString);

		if (DXGameState->AlivePlayerControllerCount <= 1)
		{
			DXGameState->MatchState = EMatchState::Ending;
		}

		if (DXGameState->AlivePlayerControllerCount <= 1)
		{
			DXGameState->MatchState = EMatchState::Ending;

			AlivePlayerControllers[0]->ClientRPCShowGameResultWidget(1);
		}

		break;
	}
	case EMatchState::Ending:
	{
		FString NotificationString = FString::Printf(TEXT("Waiting %d for returning to title."), RemainWaitingTimeForEnding);

		NotifyToAllPlayer(NotificationString);

		--RemainWaitingTimeForEnding;

		if (RemainWaitingTimeForEnding <= 0)
		{
			for (auto AliveController : AlivePlayerControllers)
			{
				AliveController->ClientRPCReturnToTitle();
			}
			for (auto DeadController : DeadPlayerControllers)
			{
				DeadController->ClientRPCReturnToTitle();
			}

			MainTimerHandle.Invalidate();
			
			FName CurrentLevelName = FName(UGameplayStatics::GetCurrentLevelName(this));
			UGameplayStatics::OpenLevel(this, CurrentLevelName, true, FString(TEXT("listen")));

			return;
		}

		break;
	}
	case EMatchState::End:
		break;
	default:
		break;
	}
}

void ADXGameModeBase::OnCharacterDead(ADXPlayerController* InController)
{
	if (IsValid(InController) == false || AlivePlayerControllers.Find(InController) == INDEX_NONE)
	{
		return;
	}

	InController->ClientRPCShowGameResultWidget(AlivePlayerControllers.Num());

	AlivePlayerControllers.Remove(InController);
	DeadPlayerControllers.Add(InController);
}