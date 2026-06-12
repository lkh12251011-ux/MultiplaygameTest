
#include "Player/DXPlayerCharacter.h"

#include "EnhancedInputSubsystems.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Gimmick/DXLandMine.h"
#include "Components/CapsuleComponent.h"
#include "Engine/DamageEvents.h"
#include "EngineUtils.h"
#include "GameFramework/GameStateBase.h"
#include "Component/DXStatusComponent.h"
#include "Component/DXHPTextWidgetComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "UI/UW_HPText.h"

ADXPlayerCharacter::ADXPlayerCharacter()
	: bCanAttack(true)
	, AttackMeleeMontagePlayTime(0.f)
	, LastStartAttackMeleeTime(0.f)
	, AttackMeleeTimeDifference(0.f)
	, MinAllowedTimeForAttackMelee(0.02f)
	
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bUseControllerDesiredRotation = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);

	SpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArm->TargetArmLength = 400.f;
	SpringArm->bUsePawnControlRotation = true;
	SpringArm->SetupAttachment(GetRootComponent());

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->bUsePawnControlRotation = false;
	Camera->SetupAttachment(SpringArm, USpringArmComponent::SocketName);

	StatusComponent = CreateDefaultSubobject<UDXStatusComponent>(TEXT("StatusComponent"));

	HPTextWidgetComponent = CreateDefaultSubobject<UDXHPTextWidgetComponent>(TEXT("HPTextWidgetComponent"));
	HPTextWidgetComponent->SetupAttachment(GetRootComponent());
	HPTextWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 100.f));
	HPTextWidgetComponent->SetWidgetSpace(EWidgetSpace::World);
	HPTextWidgetComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void ADXPlayerCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, CurrentAimPitch);
	DOREPLIFETIME(ThisClass, bCanAttack);
}

void ADXPlayerCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (IsValid(GetController()) == true)
	{
		PreviousAimPitch = CurrentAimPitch;

		FRotator ControlRotation = GetController()->GetControlRotation();
		float NormalizedPitch = FRotator::NormalizeAxis(ControlRotation.Pitch);
		CurrentAimPitch = FMath::Clamp(NormalizedPitch, -90.0f, 90.0f);
	}

	if (IsLocallyControlled() == true && PreviousAimPitch != CurrentAimPitch)
	{
		ServerRPCUpdateAimValue(CurrentAimPitch);
	}

	if (IsValid(HPTextWidgetComponent) == true && HasAuthority() == false)
	{
		FVector WidgetComponentLocation = HPTextWidgetComponent->GetComponentLocation();
		FVector LocalPlayerCameraLocation = UGameplayStatics::GetPlayerCameraManager(this, 0)->GetCameraLocation();
		HPTextWidgetComponent->SetWorldRotation(UKismetMathLibrary::FindLookAtRotation(WidgetComponentLocation, LocalPlayerCameraLocation));
	}
}

void ADXPlayerCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	UEnhancedInputComponent* EIC = CastChecked<UEnhancedInputComponent>(PlayerInputComponent);

	EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ThisClass::HandleMoveInput);

	EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &ThisClass::HandleLookInput);

	EIC->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ACharacter::Jump);
	
	EIC->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

	EIC->BindAction(LandMineAction, ETriggerEvent::Started, this, &ThisClass::HandleLandMineInput);

	EIC->BindAction(AttackMeleeAction, ETriggerEvent::Started, this, &ThisClass::HandleAttackMeleeInput);
}

void ADXPlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocallyControlled() == true)
	{
		APlayerController* PC = Cast<APlayerController>(GetController());
		checkf(IsValid(PC) == true, TEXT("PlayerController is invalid."));

		UEnhancedInputLocalPlayerSubsystem* EILPS = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
		checkf(IsValid(EILPS) == true, TEXT("EnhancedInputLocalPlayerSubsystem is invalid."));

		EILPS->AddMappingContext(InputMappingContext, 0);
	}
	if (IsValid(AttackMeleeMontage) == true)
	{
		AttackMeleeMontagePlayTime = AttackMeleeMontage->GetPlayLength();
	}
}

float ADXPlayerCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,class AController* EventInstigator, AActor* DamageCauser)
{
	UKismetSystemLibrary::PrintString(GetWorld(), FString::Printf(TEXT("TakeDamage: %f"), DamageAmount), true, true, FLinearColor::Red, 5.f);

	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	StatusComponent->ApplyDamage(ActualDamage);
	return ActualDamage;
}

void ADXPlayerCharacter::TakeBuff(float InBuffValue)
{
	if (IsValid(StatusComponent) == true)
	{
		StatusComponent->SetMaxHP(StatusComponent->GetMaxHP() + InBuffValue);
		StatusComponent->SetCurrentHP(StatusComponent->GetCurrentHP() + InBuffValue);
	}
}

void ADXPlayerCharacter::CheckAttackMeleeHit()
{
	if (IsLocallyControlled() == true)
	{
		TArray<FHitResult> OutHitResults;
		TSet<ACharacter*> DamagedCharacters;
		FCollisionQueryParams Params(NAME_None, false, this);

		const float AttackMeleekRange = 50.f;
		const float AttackMeleeRadius = 50.f;

		const FVector Forward = GetActorForwardVector();
		const FVector Start = GetActorLocation() + Forward * GetCapsuleComponent()->GetScaledCapsuleRadius();
		const FVector End = Start + GetActorForwardVector() * AttackMeleekRange;

		bool bIsHitDetected = GetWorld()->SweepMultiByChannel(OutHitResults, Start, End, FQuat::Identity, ECC_Camera, FCollisionShape::MakeSphere(AttackMeleeRadius), Params);
		if (bIsHitDetected == true)
		{
			for (auto const& OutHitResult : OutHitResults)
			{
				ACharacter* DamagedCharacter = Cast<ACharacter>(OutHitResult.GetActor());
				if (IsValid(DamagedCharacter) == true)
				{
					DamagedCharacters.Add(DamagedCharacter);
				}
			}

			FDamageEvent DamageEvent;
			for (auto const& DamagedCharacter : DamagedCharacters)
			{
				ServerRPCPerformMeleeHit(DamagedCharacter, GetWorld()->GetGameState()->GetServerWorldTimeSeconds());
			}
		}

		FColor DrawColor = bIsHitDetected ? FColor::Green : FColor::Red;
		DrawDebugAttackMelee(DrawColor, Start, End, Forward);
	}
}

void ADXPlayerCharacter::OnRep_CanAttack()
{
	if (bCanAttack == true)
	{
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
	else
	{
		GetCharacterMovement()->SetMovementMode(MOVE_None);
	}
}

void ADXPlayerCharacter::SetHPTextWidget(UUW_HPText* InHPTextWidget)
{
	UUW_HPText* HPWidget = Cast<UUW_HPText>(InHPTextWidget);
	if (IsValid(HPWidget) == true)
	{
		HPWidget->InitializeHPTextWidget(StatusComponent);
		StatusComponent->OnCurrentHPChanged.AddUObject(HPWidget, &UUW_HPText::OnCurrentHPChange);
		StatusComponent->OnMaxHPChanged.AddUObject(HPWidget, &UUW_HPText::OnMaxHPChange);
	}
}

void ADXPlayerCharacter::DrawDebugAttackMelee(const FColor& DrawColor, FVector TraceStart, FVector TraceEnd,FVector Forward)
{
	const float MeleeAttackRange = 50.f;
	const float MeleeAttackRadius = 50.f;
	FVector CapsuleOrigin = TraceStart + (TraceEnd - TraceStart) * 0.5f;
	float CapsuleHalfHeight = MeleeAttackRange * 0.5f;
	DrawDebugCapsule(GetWorld(), CapsuleOrigin, CapsuleHalfHeight, MeleeAttackRadius, FRotationMatrix::MakeFromZ(Forward).ToQuat(), DrawColor, false, 5.0f);
}

void ADXPlayerCharacter::HandleMoveInput(const FInputActionValue& InValue)
{
	if (IsValid(Controller) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("Controller is invalid."));
		return;
	}

	const FVector2D InMovementVector = InValue.Get<FVector2D>();

	const FRotator ControlRotation = Controller->GetControlRotation();
	const FRotator ControlYawRotation(0.0f, ControlRotation.Yaw, 0.0f);

	const FVector ForwardDirection = FRotationMatrix(ControlYawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(ControlYawRotation).GetUnitAxis(EAxis::Y);

	AddMovementInput(ForwardDirection, InMovementVector.X);
	AddMovementInput(RightDirection, InMovementVector.Y);
}

void ADXPlayerCharacter::HandleLookInput(const FInputActionValue& InValue)
{
	if (IsValid(Controller) == false)
	{
		UE_LOG(LogTemp, Error, TEXT("Controller is invalid."));
		return;
	}

	const FVector2D InLookVector = InValue.Get<FVector2D>();

	AddControllerYawInput(InLookVector.X);
	AddControllerPitchInput(InLookVector.Y);
}

void ADXPlayerCharacter::HandleLandMineInput(const FInputActionValue& InValue)
{
	if (IsLocallyControlled() == true)
	{
		ServerRPCSpawnLandMine();
	}
}

void ADXPlayerCharacter::HandleAttackMeleeInput(const FInputActionValue& InValue)
{
	if (true == bCanAttack && GetCharacterMovement()->IsFalling() == false)
	{
		ServerRPCAttackMelee(GetWorld()->GetGameState()->GetServerWorldTimeSeconds());

		if (HasAuthority() == false && IsLocallyControlled() == true)
		{
			PlayAttackMeleeMontage();
		}
	}
}

void ADXPlayerCharacter::ServerRPCUpdateAimValue_Implementation(const float& InAimPitchValue)
{
	CurrentAimPitch = InAimPitchValue;
}

void ADXPlayerCharacter::ServerRPCSpawnLandMine_Implementation()
{
	if (IsValid(LandMineClass) == true)
	{
		FVector SpawnedLocation = (GetActorLocation() + GetActorForwardVector() * 300.f) - FVector(0.f, 0.f, 90.f);
		ADXLandMine* SpawnedLandMine = GetWorld()->SpawnActor<ADXLandMine>(LandMineClass, SpawnedLocation, FRotator::ZeroRotator);
	}
}

bool ADXPlayerCharacter::ServerRPCSpawnLandMine_Validate()
{
	return true;
}

void ADXPlayerCharacter::ServerRPCAttackMelee_Implementation(float InStartAttackMeleeTime)
{
	AttackMeleeTimeDifference = GetWorld()->GetTimeSeconds() - InStartAttackMeleeTime;
	
	AttackMeleeTimeDifference = FMath::Clamp(AttackMeleeTimeDifference, 0.f, AttackMeleeMontagePlayTime);
	
	if (KINDA_SMALL_NUMBER < AttackMeleeMontagePlayTime - AttackMeleeTimeDifference)
	{
		bCanAttack = false;
		OnRep_CanAttack();

		FTimerHandle TimerHandle;
		GetWorld()->GetTimerManager().SetTimer(TimerHandle, FTimerDelegate::CreateLambda([&]()
			{
				bCanAttack = true;
				OnRep_CanAttack();
			}), AttackMeleeMontagePlayTime - AttackMeleeTimeDifference, false, -1.f);
	}
	
	LastStartAttackMeleeTime = InStartAttackMeleeTime;
	PlayAttackMeleeMontage();
	
	for (APlayerController* PlayerController : TActorRange<APlayerController>(GetWorld()))
	{
		if (IsValid(PlayerController) == true && GetController() != PlayerController)
		{
			ADXPlayerCharacter* OtherPlayerCharacter = Cast<ADXPlayerCharacter>(PlayerController->GetPawn());
			if (OtherPlayerCharacter)
			{
				OtherPlayerCharacter->ClientRPCPlayAttackMeleeMontage(this);
			}
		}
	}
}

bool ADXPlayerCharacter::ServerRPCAttackMelee_Validate(float InStartMeleeAttackTime)
{
	if (LastStartAttackMeleeTime == 0.0f)
	{
		return true;
	}

	return (AttackMeleeMontagePlayTime - 0.1f) < (InStartMeleeAttackTime - LastStartAttackMeleeTime);	
}

void ADXPlayerCharacter::MulticastRPCAttackMelee_Implementation()
{
	if (HasAuthority() == false && IsLocallyControlled() == false)
	{
		PlayAttackMeleeMontage();
	}
}

void ADXPlayerCharacter::ServerRPCPerformMeleeHit_Implementation(ACharacter* InDamagedCharacters,float InCheckTime)
{
	if (IsValid(InDamagedCharacters) == true)
	{
		const float AttacMeleekDamage = 10.f;
		FDamageEvent DamageEvent;
		InDamagedCharacters->TakeDamage(AttacMeleekDamage, DamageEvent, GetController(), this);
	}
}

bool ADXPlayerCharacter::ServerRPCPerformMeleeHit_Validate(ACharacter* InDamagedCharacters, float InCheckTime)
{
	return MinAllowedTimeForAttackMelee< (InCheckTime - LastStartAttackMeleeTime);
}

void ADXPlayerCharacter::ClientRPCPlayAttackMeleeMontage_Implementation(ADXPlayerCharacter* InTargetCharacter)
{
	if (IsValid(InTargetCharacter) == true)
	{
		InTargetCharacter->PlayAttackMeleeMontage();
	}
}

void ADXPlayerCharacter::PlayAttackMeleeMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (IsValid(AnimInstance) == true)
	{
		AnimInstance->StopAllMontages(0.f);
		AnimInstance->Montage_Play(AttackMeleeMontage);
	}
}