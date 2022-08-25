// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBaseActor.h"

#include "WeaponBaseComponent.h"
#include "Components/SphereComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"

// Sets default values
AWeaponBaseActor::AWeaponBaseActor()
{
	WeaponMesh = CreateDefaultSubobject<UWeaponBaseComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetCollisionProfileName(TEXT("StaticBlockPhysics"));
	WeaponMesh->SetSimulatePhysics(true);
	WeaponMesh->CanCharacterStepUpOn = ECB_No;
	WeaponMesh->SetIsReplicated(true);
	RootComponent = WeaponMesh;

	AISightSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("SightSource"));
	// AISightSource->ComponentTags.AddUnique(TEXT("Weapon"));
	
	PickUpSensor->SetupAttachment(RootComponent);

	this->Tags.AddUnique(TEXT("Weapon"));
}

void AWeaponBaseActor::BeginPlay()
{
	Super::BeginPlay();
	// AISightSource->RegisterWithPerceptionSystem();
	AISightSource->RegisterForSense(UAISense_Sight::StaticClass());
}
