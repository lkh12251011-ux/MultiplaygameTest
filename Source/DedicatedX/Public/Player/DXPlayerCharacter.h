// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "DXPlayerCharacter.generated.h"

class UCameraComponent;
class USpringArmComponent;
class UInputMappingContext;
class UInputAction;
class UAnimMontage;
class UDXStatusComponent;
class UDXHPTextWidgetComponent;
class UUW_HPText;

UCLASS()
class DEDICATEDX_API ADXPlayerCharacter : public ACharacter
{
	GENERATED_BODY()

#pragma region ACharacter Override

public:
	ADXPlayerCharacter();

	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

	virtual void BeginPlay() override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void Tick(float DeltaTime) override;

#pragma endregion

#pragma region DXPlayerCharacter Components

public:
	FORCEINLINE USpringArmComponent* GetSpringArm() const { return SpringArm; }

	FORCEINLINE UCameraComponent* GetCamera() const { return Camera; }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DXPlayerCharacter|Components")
	TObjectPtr<USpringArmComponent> SpringArm;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DXPlayerCharacter|Components")
	TObjectPtr<UCameraComponent> Camera;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DXPlayerCharacter|Components")
	TObjectPtr<UDXStatusComponent> StatusComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DXPlayerCharacter|Components")
	TObjectPtr<UDXHPTextWidgetComponent> HPTextWidgetComponent;


#pragma endregion

#pragma region Input

public:
	float GetCurrentAimPitch() const { return CurrentAimPitch; }

private:
	void HandleMoveInput(const FInputActionValue& InValue);

	void HandleLookInput(const FInputActionValue& InValue);

	void HandleLandMineInput(const FInputActionValue& InValue);

	void HandleAttackMeleeInput(const FInputActionValue& InValue);

	UFUNCTION(Server, Unreliable)
	void ServerRPCUpdateAimValue(const float& InAimPitchValue);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DXPlayerCharacter|Input")
	TObjectPtr<UInputMappingContext> InputMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DXPlayerCharacter|Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DXPlayerCharacter|Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DXPlayerCharacter|Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DXPlayerCharacter|Input")
	TObjectPtr<UInputAction> LandMineAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "DXPlayerCharacter|Input")
	TObjectPtr<UInputAction> AttackMeleeAction;

	UPROPERTY(Replicated)
	float CurrentAimPitch = 0.f;

	float PreviousAimPitch = 0.f;

#pragma endregion

#pragma region LandMine

private:
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRPCSpawnLandMine();

protected:

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AActor> LandMineClass;

#pragma endregion

#pragma region Attack

public:
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	void CheckAttackMeleeHit();

private:
	void DrawDebugAttackMelee(const FColor& DrawColor, FVector TraceStart, FVector TraceEnd, FVector Forward);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRPCAttackMelee(float InStartAttackMeleeTime);

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastRPCAttackMelee();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerRPCPerformMeleeHit(ACharacter* InDamagedCharacters, float InCheckTime);

	UFUNCTION(Client, Unreliable)
	void ClientRPCPlayAttackMeleeMontage(ADXPlayerCharacter* InTargetCharacter);

	UFUNCTION()
	void OnRep_CanAttack();

	void PlayAttackMeleeMontage();

protected:
	UPROPERTY(ReplicatedUsing = OnRep_CanAttack)
	uint8 bCanAttack : 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UAnimMontage> AttackMeleeMontage;

	float AttackMeleeMontagePlayTime;

	float LastStartAttackMeleeTime;

	float AttackMeleeTimeDifference;

	float MinAllowedTimeForAttackMelee;

#pragma endregion

#pragma region HPWidget

public:
	void SetHPTextWidget(UUW_HPText* InHPTextWidget);

	void TakeBuff(float InBuffValue);

#pragma endregion

};