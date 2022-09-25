// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BackpackItemInterface.h"
#include "Components/SkeletalMeshComponent.h"
#include "BackpackComponent.generated.h"

/**
 * 
 */
UCLASS()
class NETSHOOTGAME_API UBackpackComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Replicated, Category="Backpack")
	TArray<FBackpackItemInfo> BagContainer;
	
	UFUNCTION(BlueprintCallable)
	bool AddToBag(AActor* Actor);
	
	UFUNCTION(BlueprintCallable)
	void RemoveFromBag(int32 Index, int32 Num = 1);

	UFUNCTION(BlueprintCallable)
	FORCEINLINE int32 Num() { return BagContainer.Num(); }
	
private:
	AActor* SpawnActorByInfo(FBackpackItemInfo& Info, const FTransform& Transform) const;
	
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;
};
