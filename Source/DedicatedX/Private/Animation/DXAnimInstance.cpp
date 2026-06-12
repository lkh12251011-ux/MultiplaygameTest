// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/DXAnimInstance.h"

#include "Player/DXPlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

void UDXAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwnerCharacter = Cast<ADXPlayerCharacter>(GetOwningActor());
	if (IsValid(OwnerCharacter) == true)
	{
		OwnerCharacterMovementComponent = OwnerCharacter->GetCharacterMovement();
	}
}

void UDXAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (IsValid(OwnerCharacter) == false || IsValid(OwnerCharacterMovementComponent) == false)
	{
		return;
	}

	Velocity = OwnerCharacterMovementComponent->Velocity;
	GroundSpeed = FVector(Velocity.X, Velocity.Y, 0.f).Size();
	bShouldMove = ((OwnerCharacterMovementComponent->GetCurrentAcceleration().IsNearlyZero()) == false) && (3.f < GroundSpeed);
	bIsFalling = OwnerCharacterMovementComponent->IsFalling();
	AimPitch = OwnerCharacter->GetCurrentAimPitch();
}

void UDXAnimInstance::AnimNotify_CheckAttackMeleeHit()
{
	if (IsValid(OwnerCharacter) == true)
	{
		OwnerCharacter->CheckAttackMeleeHit();
	}
}