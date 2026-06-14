

#include "Player/DXPlayerController.h"

#include "Blueprint/UserWidget.h"
#include "Net/UnrealNetwork.h"
#include "GameMode/DXGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "UI/UW_GameResult.h"
#include "Components/TextBlock.h"

void ADXPlayerController::BeginPlay()
{
	if (IsLocalController() == false)
	{
		return;
	}

	FInputModeGameOnly GameOnly;
	SetInputMode(GameOnly);

	if (IsValid(NotificationTextUIClass) == true)
	{
		UUserWidget* NotificationTextUI = CreateWidget<UUserWidget>(this, NotificationTextUIClass);
		if (IsValid(NotificationTextUI) == true)
		{
			NotificationTextUI->AddToViewport(1);

			NotificationTextUI->SetVisibility(ESlateVisibility::Visible);
		}
	}
}

void ADXPlayerController::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, NotificationText);
}

void ADXPlayerController::OnCharacterDead()
{
	ADXGameModeBase* GameMode = Cast<ADXGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (HasAuthority() == true && IsValid(GameMode) == true)
	{
		GameMode->OnCharacterDead(this);
	}
}

void ADXPlayerController::ClientRPCShowGameResultWidget_Implementation(int32 InRanking)
{
	if (IsLocalController() == true)
	{
		if (IsValid(GameResultUIClass) == true)
		{
			UUW_GameResult* GameResultUI = CreateWidget<UUW_GameResult>(this, GameResultUIClass);
			if (IsValid(GameResultUI) == true)
			{
				GameResultUI->AddToViewport(3);

				FString GameResultString = FString::Printf(TEXT("%s"), InRanking == 1 ? TEXT("Winner Winner!") : TEXT("Looser..."));
				GameResultUI->ResultText->SetText(FText::FromString(GameResultString));

				FString RankingString = FString::Printf(TEXT("#%02d"), InRanking);
				GameResultUI->RankingText->SetText(FText::FromString(RankingString));

				FInputModeUIOnly Mode;
				Mode.SetWidgetToFocus(GameResultUI->GetCachedWidget());
				SetInputMode(Mode);

				bShowMouseCursor = true;
			}
		}
	}
}

void ADXPlayerController::ClientRPCReturnToTitle_Implementation()
{
	if (IsLocalController() == true)
	{ 
		UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("Title")), true);
	}
}