
#include "Gimmick/DXBox.h"
#include "DedicatedX.h"
#include "Net/UnrealNetwork.h"

ADXBox::ADXBox()
	: ServerRotationYaw(0.0f)
	, RotationSpeed(30.0f)
	, AccDeltaSecondSinceReplicated(0.0f)
	, NetCullDistance(1000.f)
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(SceneRoot);
	Mesh->SetRelativeLocation(FVector(-50.f, -50.f, 50.f));

	const static float BoxActorNetUpdateFrequency = 1.f;
	SetNetUpdateFrequency(BoxActorNetUpdateFrequency);
	NetUpdatePeriod = 1 / GetNetUpdateFrequency();

	SetNetCullDistanceSquared(NetCullDistance * NetCullDistance);
}

void ADXBox::BeginPlay()
{
	Super::BeginPlay();

	DX_LOG_ROLE(LogDXNet, Log, TEXT(""));
}

void ADXBox::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ThisClass, ServerRotationYaw);
}

void ADXBox::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (HasAuthority() == true)
	{
		AddActorLocalRotation(FRotator(0.f, RotationSpeed * DeltaSeconds, 0.f));
		ServerRotationYaw = RootComponent->GetComponentRotation().Yaw;
	}
	else
	{
		if (NetUpdatePeriod < KINDA_SMALL_NUMBER)
		{
			return;
		}

		AccDeltaSecondSinceReplicated += DeltaSeconds;
		const float LerpRatio = FMath::Clamp(AccDeltaSecondSinceReplicated / NetUpdatePeriod, 0.f, 1.f);

		const float NextServerRotationYaw = ServerRotationYaw + RotationSpeed * NetUpdatePeriod;

		const float EstimatedClientRotationYaw = FMath::Lerp(ServerRotationYaw, NextServerRotationYaw, LerpRatio);
		SetActorRotation(FRotator(0.f, EstimatedClientRotationYaw, 0.f));
	}

	DrawDebugSphere(GetWorld(), GetActorLocation(), NetCullDistance / 2.f, 16, FColor::Green, false, -1.f);
}

bool ADXBox::IsNetRelevantFor(const AActor* RealViewer, const AActor* ViewTarget, const FVector& SrcLocation) const
{
	bool bIsNetRelevant = Super::IsNetRelevantFor(RealViewer, ViewTarget, SrcLocation);

	if (false == bIsNetRelevant)
	{
		DX_LOG_NET(LogDXNet, Log, TEXT("%s is not relevant for(%s, %s)"), *GetName(), *RealViewer->GetName(), *ViewTarget->GetName());
	}

	return bIsNetRelevant;
}

void ADXBox::OnRep_ServerRotationYaw()
{
	DX_LOG_NET(LogDXNet, Log, TEXT("OnRep_ServerRotationYaw(): %f"), ServerRotationYaw);

	SetActorRotation(FRotator(0.f, ServerRotationYaw, 0.f));

	AccDeltaSecondSinceReplicated = 0.f;
}


