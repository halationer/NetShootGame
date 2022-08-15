// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBaseActor.h"

#include "WeaponBaseComponent.h"
#include "Components/SphereComponent.h"

// Sets default values
AWeaponBaseActor::AWeaponBaseActor()
{
	WeaponMesh = CreateDefaultSubobject<UWeaponBaseComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetCollisionProfileName(TEXT("StaticBlockPhysics"));
	WeaponMesh->SetSimulatePhysics(true);
	WeaponMesh->CanCharacterStepUpOn = ECB_No;
	WeaponMesh->SetIsReplicated(true);
	RootComponent = WeaponMesh;
	
	PickUpSensor->SetupAttachment(RootComponent);
}
