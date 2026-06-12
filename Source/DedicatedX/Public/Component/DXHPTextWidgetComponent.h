// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/WidgetComponent.h"
#include "DXHPTextWidgetComponent.generated.h"

/**
 * 
 */
UCLASS()
class DEDICATEDX_API UDXHPTextWidgetComponent : public UWidgetComponent
{
	GENERATED_BODY()
	
public:
	virtual void InitWidget() override;

};
