// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameState.h"
#include "NetShootGameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FAllReadyDelegate);

/**
 * 
 */
UCLASS()
class NETSHOOTGAME_API ANetShootGameState : public AGameState
{
	GENERATED_BODY()

protected:
	
	UPROPERTY(BlueprintGetter=GetAllPlayersReady, Replicated, Category=GameState)
	uint32 bAllPlayersReady: 1;

public:

	ANetShootGameState();
	
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void UpdateAllPlayersReady();

	UFUNCTION(BlueprintPure, BlueprintGetter, Category=GameState)
	FORCEINLINE bool GetAllPlayersReady() const { return bAllPlayersReady; }
	
	UPROPERTY(BlueprintAssignable)
	FAllReadyDelegate AllReadyDelegate;

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;
};
