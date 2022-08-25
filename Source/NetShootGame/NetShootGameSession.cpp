// Fill out your copyright notice in the Description page of Project Settings.


#include "NetShootGameSession.h"
#include "Engine.h"
#include "NetShootGameGameMode.h"


ANetShootGameSession::ANetShootGameSession()
{
	RoomName = TEXT("DefualtRoom");
}

FString ANetShootGameSession::ApproveLogin(const FString& Options)
{
	UWorld* const World = GetWorld();
	check(World);

	ANetShootGameGameMode* const GameMode = Cast<ANetShootGameGameMode>(World->GetAuthGameMode());
	check(GameMode);

	if(GameMode->GetNumPlayers() >= GameMode->RoomMaxPlayerNum)
	{
		FString ErrorMessage = TEXT("room_player_full");
		GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Orange, ErrorMessage);
		
		return ErrorMessage;
	}
	
	return Super::ApproveLogin(Options);
}
