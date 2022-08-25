// Fill out your copyright notice in the Description page of Project Settings.


#include "GrenadeComponent.h"

#include "Components/PointLightComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"
#include "PhysicsEngine/RadialForceComponent.h"
#include "Niagara/Public/NiagaraComponent.h"
#include "Niagara/Public/NiagaraFunctionLibrary.h"

UGrenadeComponent::UGrenadeComponent()
{
	Damage = 130.0f;
	DamageRadius = 500.0f;
	DamageReduction = 0.2f;
	ExplodeDelayTime = 5.0f;
	bIsStart = false;
	
	ExplodeNiagaraSystem = Cast<UNiagaraSystem>(
		StaticLoadObject(UNiagaraSystem::StaticClass(), nullptr, TEXT("NiagaraSystem'/Game/AWeapons/GrenadeExplode.GrenadeExplode'")));
	
	SkeletalMesh = Cast<USkeletalMesh>(
		StaticLoadObject(USkeletalMesh::StaticClass(), nullptr, TEXT("SkeletalMesh'/Game/FPS_Weapon_Bundle/Weapons/Meshes/G67_Grenade/SK_G67.SK_G67'")));
	
	SignalLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("SignalLight"));
	SignalLight->SetIntensity(10.0);
	SignalLight->SetLightColor(FLinearColor::Green);
	SignalLight->SetAttenuationRadius(2.0f);
	SignalLight->SetVisibility(true);
	SignalLight->SetupAttachment(this, TEXT("SignalLight"));
	SignalLight->SetIsReplicated(true);
	
	RadialForce = CreateDefaultSubobject<URadialForceComponent>(TEXT("ExplodeForce"));
	RadialForce->Radius = DamageRadius;
	RadialForce->ImpulseStrength = Damage * 100.f;
	RadialForce->SetupAttachment(this);
	RadialForce->SetIsReplicated(true);
}

void UGrenadeComponent::StartExplode()
{
	bIsStart = true;
	Rep_OnExplodeStart();
	MARK_PROPERTY_DIRTY_FROM_NAME(UGrenadeComponent, bIsStart, this);
	
	UWorld* World = GetWorld();
	if(World)
	{
		// UE_LOG(LogTemp, Warning, TEXT("StartExplode: %f, %f, %f"), GetComponentLocation().X, GetComponentLocation().Y, GetComponentLocation().Z);
		World->GetTimerManager().SetTimer(ExplodeDelayTimer, this, &UGrenadeComponent::Explode, ExplodeDelayTime);
	}
}

void UGrenadeComponent::Rep_OnExplodeStart()
{
	if(bIsStart)
		SignalLight->SetLightColor(FLinearColor::Red);
}

void UGrenadeComponent::Explode()
{
	UE_LOG(LogTemp, Warning, TEXT("Explode"));
	
	UWorld* World = GetWorld();
	if(World)
	{
		World->GetTimerManager().ClearTimer(ExplodeDelayTimer);
		
		FVector ExplodeLocation = GetComponentLocation();
		TArray<AActor*> ActorsToIgnore;
		TArray<FHitResult> HitResults;
		UKismetSystemLibrary::SphereTraceMulti(
			GetOwner(), ExplodeLocation, ExplodeLocation, DamageRadius, SELF_DEFINED_EXPLODE_TRACE_CHANNEL, false,
			ActorsToIgnore, /*EDrawDebugTrace::ForDuration*/ EDrawDebugTrace::None, HitResults, true);

		// Put the pawn to the end, so the explosion can first effect other actors
		for(uint32 Index = 0, TailIndex = HitResults.Num(); Index < TailIndex; ++Index)
		{
			APawn* Pawn = Cast<APawn>(HitResults[Index].Actor);
			if(Pawn)
			{
				HitResults.Swap(Index, --TailIndex);
			}
		}
	
		// the first owner is the GrenadeActor, the second owner is probably the NetShootCharacter
		AActor* const DamageCauser = GetOwner()->GetOwner();
		AController* const Instigator = DamageCauser->GetInstigatorController();

		// for each hit, process it
		for(auto It = HitResults.CreateConstIterator(); It; ++It)
		{
			FHitResult HitResult = *It;
			if(HitResult.Actor == nullptr || !HitResult.Actor.IsValid())
			{
				UE_LOG(LogTemp, Warning, TEXT("Explode continue"));
				continue;
			}
			
			FVector HitDirection = HitResult.ImpactPoint - ExplodeLocation;
			float DistanceFromCenter = FMath::Abs(HitDirection.Size());
			HitDirection.Normalize();
			float DamageAmount = Damage - DamageReduction * DistanceFromCenter;
			UE_LOG(LogTemp, Warning, TEXT("%s, %s: %f, %f"), GetData(HitResult.Actor->GetName()), GetData(HitResult.Component->GetName()), DistanceFromCenter, DamageAmount);

			const FPointDamageEvent DamageEvent(DamageAmount, HitResult, HitDirection, UDamageType::StaticClass());
			HitResult.Actor->TakeDamage(DamageAmount, DamageEvent, Instigator, DamageCauser);
		}

		PlayExplodeNiagaraAnimation();
		RadialForce->FireImpulse();
	}
	
	ExplodeFinishDelegate.Broadcast();
}

void UGrenadeComponent::PlayExplodeNiagaraAnimation_Implementation()
{
	UNiagaraComponent* NiagaraComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ExplodeNiagaraSystem, GetComponentLocation());
}

void UGrenadeComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;
	
	DOREPLIFETIME_WITH_PARAMS_FAST(UGrenadeComponent, bIsStart, SharedParams);
}
