// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "NetShootGamePlayerController.generated.h"

/**
 * 
 */
UCLASS()
class NETSHOOTGAME_API ANetShootGamePlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void ShowPlayerState();

	UFUNCTION(Client, Reliable)
	void BindShowPlayerState_Client();

	UFUNCTION(Client, Reliable)
	void UnBindShowPlayerState_Client();
};
