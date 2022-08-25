// Fill out your copyright notice in the Description page of Project Settings.


#include "ItemBaseActor.h"

#include "Components/SphereComponent.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"
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

	bCanSensed = true;
}

void AItemBaseActor::InitCannotSensed()
{
	bCanSensed = false;
	PickUpSensor->SetCollisionEnabled(ECollisionEnabled::Type::NoCollision);
}

void AItemBaseActor::SetCanSensed_Implementation(const bool CanSensed)
{
	bCanSensed = CanSensed;
	PickUpSensor->SetCollisionEnabled(CanSensed ? ECollisionEnabled::Type::QueryOnly : ECollisionEnabled::Type::NoCollision);
	MARK_PROPERTY_DIRTY_FROM_NAME(AItemBaseActor, bCanSensed, this);
}

void AItemBaseActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
	if(bCanSensed)
	{
		Super::NotifyActorBeginOverlap(OtherActor);
    
        ANetShootGameCharacter* NetShootGameCharacter = Cast<ANetShootGameCharacter>(OtherActor);
        if(NetShootGameCharacter)
        {
        	NetShootGameCharacter->CloseToItem(this);
        }
	}
}

void AItemBaseActor::NotifyActorEndOverlap(AActor* OtherActor)
{
	if(bCanSensed)
	{
		Super::NotifyActorEndOverlap(OtherActor);
        ANetShootGameCharacter* NetShootGameCharacter = Cast<ANetShootGameCharacter>(OtherActor);
        if(NetShootGameCharacter)
        {
        	NetShootGameCharacter->AwayFromItem(this);
        }
	}
}

void AItemBaseActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;
	
	DOREPLIFETIME_WITH_PARAMS_FAST(AItemBaseActor, bCanSensed, SharedParams);
}
