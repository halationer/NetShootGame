// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBaseComponent.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Components/SpotLightComponent.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

UWeaponBaseComponent::UWeaponBaseComponent()
{
	bIsAuto = false;
	FireRate = 0.16f;
	FireDistance = 5000.0f;
	FireBaseDamage = 10.0f;
	FireSocketName = TEXT("Muzzle");

	bIsFire = false;
	bWithTarget = false;

	FireNiagaraSystem = Cast<UNiagaraSystem>(
		StaticLoadObject(UNiagaraSystem::StaticClass(), nullptr, TEXT("NiagaraSystem'/Game/AWeapons/WeaponFire.WeaponFire'")));
	
	SkeletalMesh = Cast<USkeletalMesh>(
		StaticLoadObject(USkeletalMesh::StaticClass(), nullptr, TEXT("SkeletalMesh'/Game/FPS_Weapon_Bundle/Weapons/Meshes/Ka47/SK_KA47.SK_KA47'")));
	
	FireLight = CreateDefaultSubobject<USpotLightComponent>(TEXT("FireLight"));
	FireLight->SetVisibility(false);
	FireLight->SetupAttachment(this, FireSocketName);
}

void UWeaponBaseComponent::SetWeaponAttribute_Implementation(const FWeaponAttribute& NewAttribute)
{
	WeaponAttribute = NewAttribute;
	MARK_PROPERTY_DIRTY_FROM_NAME(UWeaponBaseComponent, WeaponAttribute, this);
	OnAttributeChange();
}

void UWeaponBaseComponent::OnAttributeChange()
{
	SetSkeletalMesh(WeaponAttribute.SkeletonMesh, true);
}

void UWeaponBaseComponent::FireStart()
{
	if(bIsAuto)
	{
		AutoFireStart();
	}
	else
	{
		if(!bIsFire)
		{
			FireOnce();
		}
	}
}

void UWeaponBaseComponent::FireStop()
{
	AutoFireStop();
}

FTransform UWeaponBaseComponent::GetFireStartTransform() const
{
	return GetSocketTransform(FireSocketName);
}

void UWeaponBaseComponent::UpdateTargetLocation_Implementation(const FVector& NewTargetLocation)
{
	TargetLocation = NewTargetLocation;
}

void UWeaponBaseComponent::SetWithTarget_Implementation(const bool NewWithTarget)
{
	bWithTarget = NewWithTarget;
}

void UWeaponBaseComponent::MulticastFireOnce_Implementation()
{
	FireOnceDelegate.Broadcast();
	
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FireNiagaraSystem, GetSocketLocation(FireSocketName));
}

void UWeaponBaseComponent::FireOnce()
{
	bIsFire = true;
    
    UWorld* World = GetWorld();
    if(World)
    {
        World->GetTimerManager().SetTimer(FireOnceTimerHandle, this, &UWeaponBaseComponent::UnlockFire, FireRate * 0.9);
        
        FTransform FireStartTransform = GetFireStartTransform();
        const FVector TraceStart = FireStartTransform.GetLocation();
        FVector TraceEnd;
    	if(bWithTarget)
    	{
    		FVector TraceDirection = TargetLocation - TraceStart;
    		TraceDirection.Normalize();
    		TraceEnd = TraceStart +  TraceDirection * FireDistance;
    	}
        else
        {
    		TraceEnd = TraceStart + (FireStartTransform.GetUnitAxis(EAxis::X) * FireDistance);
        }
        const FCollisionQueryParams QueryParams(TEXT("WeaponFire"), false, GetOwner());

		// GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Blue, TEXT("SHOOT"));
    	MulticastFireOnce();
    	
        FHitResult Hit;
        if (World->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Pawn, QueryParams))
        {
        	if (Hit.Actor.IsValid())
        	{
        		// GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::Blue, FString::Printf(TEXT("SHOOT DAMAGE! %s"), GetData(Hit.Actor->GetName())));
        		
        		const float DamageAmount = FireBaseDamage;
        		const FVector ShotFromDirection = (TraceEnd - TraceStart).GetSafeNormal();
        		const FPointDamageEvent DamageEvent(DamageAmount, Hit, ShotFromDirection, UDamageType::StaticClass());
        		Hit.Actor->TakeDamage(DamageAmount, DamageEvent, GetOwner()->GetInstigatorController(), GetOwner());
        	}
        }
    }
}

void UWeaponBaseComponent::AutoFireStart()
{
	UWorld* World = GetWorld();
	if(World)
	{
		World->GetTimerManager().SetTimer(AutoFireTimerHandle, this, &UWeaponBaseComponent::FireOnce, FireRate, true, 0);
	}
}

void UWeaponBaseComponent::AutoFireStop()
{
	UWorld* World = GetWorld();
	if(World)
	{
		World->GetTimerManager().ClearTimer(AutoFireTimerHandle);
	}
}

/////////////////////////////////////
//Replicate
void UWeaponBaseComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(UWeaponBaseComponent, WeaponAttribute, SharedParams);


	DOREPLIFETIME(UWeaponBaseComponent, bIsFire);
}
