// Fill out your copyright notice in the Description page of Project Settings.


#include "UI/UW_TitleLayout.h"

#include "Components/Button.h"
#include "Components/EditableText.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Controller/DXTitlePlayerController.h"

UUW_TitleLayout::UUW_TitleLayout(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
}

void UUW_TitleLayout::NativeConstruct()
{
	PlayButton.Get()->OnClicked.AddDynamic(this, &ThisClass::OnPlayButtonClicked);
	ExitButton.Get()->OnClicked.AddDynamic(this, &ThisClass::OnExitButtonClicked);
}

void UUW_TitleLayout::OnPlayButtonClicked()
{
	ADXTitlePlayerController* PlayerController = GetOwningPlayer<ADXTitlePlayerController>();
	if (IsValid(PlayerController) == true)
	{
		FText ServerIP = ServerIPEditableText->GetText();
		PlayerController->JoinServer(ServerIP.ToString());
	}
}

void UUW_TitleLayout::OnExitButtonClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}
