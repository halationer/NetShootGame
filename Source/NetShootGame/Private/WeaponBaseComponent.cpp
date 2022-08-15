// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBaseComponent.h"

#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

FWeaponAttribute FWeaponAttribute::NONE = FWeaponAttribute();

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

void UWeaponBaseComponent::Fire(const FTransform& MuzzleTransform, float TraceDistance)
{
	const FVector TraceStart = MuzzleTransform.GetLocation();
	const FVector TraceEnd = TraceStart + (MuzzleTransform.GetUnitAxis(EAxis::X) * TraceDistance);
	const FCollisionQueryParams QueryParams(TEXT("WeaponFire"), false, GetOwner());

	FHitResult Hit;
	if (GetWorld()->LineTraceSingleByChannel(Hit, TraceStart, TraceEnd, ECC_Visibility, QueryParams))
	{
		if (Hit.Actor.IsValid())
		{
			const float DamageAmount = 1.0f;
			const FVector ShotFromDirection = (TraceEnd - TraceStart).GetSafeNormal();
			const FPointDamageEvent DamageEvent(DamageAmount, Hit, ShotFromDirection, UDamageType::StaticClass());
			Hit.Actor->TakeDamage(DamageAmount, DamageEvent, GetOwner()->GetInstigatorController(), GetOwner());
		}
	}
}

void UWeaponBaseComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;

	DOREPLIFETIME_WITH_PARAMS_FAST(UWeaponBaseComponent, WeaponAttribute, SharedParams);
}
