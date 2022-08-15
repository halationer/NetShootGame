// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemBaseActor.h"
#include "WeaponBaseActor.generated.h"

UCLASS()
class NETSHOOTGAME_API AWeaponBaseActor : public AItemBaseActor
{
	GENERATED_BODY()

public:
	
	/** SkeletalMesh Of the Weapon */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Model, meta = (AllowPrivateAccess = "true"))
	class UWeaponBaseComponent* WeaponMesh;
	
	// Sets default values for this actor's properties
	AWeaponBaseActor();

};
