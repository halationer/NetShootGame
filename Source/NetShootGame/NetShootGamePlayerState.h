// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "NetShootGamePlayerState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FReadyChangeDelegate);

// if a delegate use few bind functions, choose "sparse" to opitmize memory
// DECLARE_DYNAMIC_MULTICAST_SPARSE_DELEGATE()

/**
 * 
 */
UCLASS()
class NETSHOOTGAME_API ANetShootGamePlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Replicated, Category=GamePlay)
	int32 DestroyTargetNum;

	UPROPERTY(BlueprintReadWrite, Replicated, Category=GamePlay)
	int32 KillNum;

	UPROPERTY(BlueprintReadWrite, Replicated, Category=GamePlay)
	int32 DeathNum;

	UPROPERTY(BlueprintReadWrite, Replicated, Category=PlayerState)
	FString PlayerName;

private:
	static int32 player_name_id;
	
protected:
	
	UPROPERTY(BlueprintGetter=IsReady, ReplicatedUsing=Rep_OnReadyChange, Category=PlayerState)
	uint32 bIsReady : 1;

public:
	
	UFUNCTION(BlueprintPure, BlueprintGetter)
	FORCEINLINE bool IsReady() const { return bIsReady; }

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void SetReady(const bool IsReady);

	UFUNCTION()
	void Rep_OnReadyChange();

	UPROPERTY(BlueprintAssignable)
	FReadyChangeDelegate ReadyChangeDelegate;

	virtual void BeginPlay() override;
	
	ANetShootGamePlayerState();
	
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;
};
