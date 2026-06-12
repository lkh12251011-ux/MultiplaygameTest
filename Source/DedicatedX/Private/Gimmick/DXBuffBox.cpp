// Fill out your copyright notice in the Description page of Project Settings.


#include "Gimmick/DXBuffBox.h"

#include "Components/BoxComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Player/DXPlayerCharacter.h"

ADXBuffBox::ADXBuffBox()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;

	TriggerBox = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerBox"));
	SetRootComponent(TriggerBox);
	TriggerBox->SetBoxExtent(FVector(40.0f, 42.0f, 30.0f));

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(TriggerBox);
	Mesh->SetRelativeLocation(FVector(0.0f, -3.5f, -30.0f));
	Mesh->SetCollisionProfileName(TEXT("NoCollision"));

	ParticleEffect = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleEffect"));
	ParticleEffect->SetupAttachment(TriggerBox);
	ParticleEffect->bAutoActivate = false;
}

void ADXBuffBox::BeginPlay()
{
	Super::BeginPlay();

	TriggerBox->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnOverlapBegin);
}

void ADXBuffBox::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
	UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepHitResult)
{
	ParticleEffect->Activate(true);
	Mesh->SetHiddenInGame(true);
	SetActorEnableCollision(false);
	ParticleEffect->OnSystemFinished.AddDynamic(this, &ThisClass::OnEffectFinished);

	ADXPlayerCharacter* OverlappingCharacter = Cast<ADXPlayerCharacter>(OtherActor);
	if (IsValid(OverlappingCharacter) == true)
	{
		OverlappingCharacter->TakeBuff(50.f);
	}
}

void ADXBuffBox::OnEffectFinished(UParticleSystemComponent* ParticleSystem)
{
	if (HasAuthority() == true)
	{
		Destroy();
	}
}

