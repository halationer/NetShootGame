// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameSession.h"
#include "NetShootGameSession.generated.h"

/**
 * 
 */
UCLASS()
class NETSHOOTGAME_API ANetShootGameSession : public AGameSession
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite)
	FString RoomName;

	ANetShootGameSession();

	// ApproveLogin is called by PreLogin in GameModeBase
	virtual FString ApproveLogin(const FString& Options) override;
};
