// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemBaseActor.h"

#include "Components/SphereComponent.h"
#include "NetShootGame/NetShootGameCharacter.h"

// Sets default values
AItemBaseActor::AItemBaseActor()
{
	this->SetReplicates(true);
	this->SetReplicatingMovement(true);

	PickUpSensor = CreateDefaultSubobject<USphereComponent>(TEXT("PickUpSensor"));
	PickUpSensor->InitSphereRadius(80.f);
	PickUpSensor->SetCollisionProfileName(TEXT("Sensor"));
	PickUpSensor->CanCharacterStepUpOn = ECB_No;
	PickUpSensor->SetIsReplicated(true);
}

void AItemBaseActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
	Super::NotifyActorBeginOverlap(OtherActor);

	ANetShootGameCharacter* NetShootGameCharacter = Cast<ANetShootGameCharacter>(OtherActor);
	if(NetShootGameCharacter)
	{
		NetShootGameCharacter->CloseToItem(this);
	}
}

void AItemBaseActor::NotifyActorEndOverlap(AActor* OtherActor)
{
	Super::NotifyActorEndOverlap(OtherActor);
	ANetShootGameCharacter* NetShootGameCharacter = Cast<ANetShootGameCharacter>(OtherActor);
	if(NetShootGameCharacter)
	{
		NetShootGameCharacter->AwayFromItem(this);
	}
}
