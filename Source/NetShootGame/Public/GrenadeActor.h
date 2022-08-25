// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ItemBaseActor.h"
#include "GrenadeActor.generated.h"

/**
 * 
 */
UCLASS()
class NETSHOOTGAME_API AGrenadeActor : public AItemBaseActor
{
	GENERATED_BODY()
	
	/** SkeletalMesh Of the Weapon */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Model, meta = (AllowPrivateAccess = "true"))
	class UGrenadeComponent* GrenadeMesh;

public:
	
	// Sets default values for this actor's properties
	AGrenadeActor();

	FORCEINLINE class UGrenadeComponent* GetGrenadeMesh() const { return GrenadeMesh; }
	

private:
	
	UFUNCTION()
	void OnExplodeFinish();
};
