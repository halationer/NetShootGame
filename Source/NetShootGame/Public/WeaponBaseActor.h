// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemBaseActor.h"
#include "WeaponBaseActor.generated.h"

UCLASS()
class NETSHOOTGAME_API AWeaponBaseActor : public AItemBaseActor
{
private:
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = AI, meta = (AllowPrivateAccess = "true"))
	class UAIPerceptionStimuliSourceComponent* AISightSource;
	
protected:

	virtual void BeginPlay() override;
	
public:
	
	/** SkeletalMesh Of the Weapon */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Model, meta = (AllowPrivateAccess = "true"))
	class UWeaponBaseComponent* WeaponMesh;
	
	// Sets default values for this actor's properties
	AWeaponBaseActor();


	/////////////////////// IBackpackItemInterface //////////////////////////

	virtual bool ActorEqualToInfo(AActor* Actor, FBackpackItemInfo& Info) override;
    virtual FBackpackItemInfo GenerateBackpackItemInfo_Implementation(AActor* Actor) override;
    virtual void InitItemBeforeSpawn_Implementation(FBackpackItemInfo& Info, AActor* Actor) override;

};
