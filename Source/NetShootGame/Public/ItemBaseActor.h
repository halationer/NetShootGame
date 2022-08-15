// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ItemBaseActor.generated.h"

UCLASS(Abstract)
class NETSHOOTGAME_API AItemBaseActor : public AActor
{
	GENERATED_BODY()
	
protected:
	/** Collision for Pawn to Pick up */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Sensor, meta = (AllowPrivateAccess = "true"))
	class USphereComponent* PickUpSensor;
	
public:	
	// Sets default values for this actor's properties
	AItemBaseActor();

	virtual void NotifyActorBeginOverlap(AActor* OtherActor) override;
	virtual void NotifyActorEndOverlap(AActor* OtherActor) override;
};
