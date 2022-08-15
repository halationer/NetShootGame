// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameMode.h"
#include "GameFramework/PlayerStart.h"
#include "NetShootGameGameMode.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FRoomPlayerFullDelegate);

UCLASS(minimalapi)
class ANetShootGameGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	ANetShootGameGameMode();

	UPROPERTY(EditAnywhere,BlueprintReadWrite, Category=GameMode)
	int32 RoomMaxPlayerNum;
	
	virtual void PostLogin(APlayerController* NewPlayer) override;

	APlayerStart* GetPlayerStart(const int32 TargetIndex) const;

	UFUNCTION(BlueprintPure, Category=GameMode)
	bool IsAllPlayersReady() const;

	UFUNCTION(BlueprintCallable, Category=GameMode)
	void StartGame();
};



