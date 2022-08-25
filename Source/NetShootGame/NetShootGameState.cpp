// Fill out your copyright notice in the Description page of Project Settings.


#include "NetShootGameState.h"

#include "NetShootGamePlayerState.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

ANetShootGameState::ANetShootGameState()
{
	bAllPlayersReady = false;
}

void ANetShootGameState::UpdateAllPlayersReady_Implementation()
{
	// for each player, check it's ready state 
	bool bTempAllReady = true;
	TArray<APlayerState*> GamePlayerStates = PlayerArray;
	for (APlayerState* GamePlayerState : GamePlayerStates)
	{
		ANetShootGamePlayerState* NetShootGamePlayerState = Cast<ANetShootGamePlayerState>(GamePlayerState);
		bTempAllReady = bTempAllReady && NetShootGamePlayerState->IsReady();
        	
		UE_LOG(LogTemp, Warning, TEXT("Player is ready ? %s"), NetShootGamePlayerState->IsReady() ? TEXT("yes") : TEXT("No"));
	}
	UE_LOG(LogTemp, Warning, TEXT("Total %d player state"), GamePlayerStates.Num());

	// if change then update, and change widget (play game button)
	if(bAllPlayersReady != bTempAllReady)
	{
		// use push model
		MARK_PROPERTY_DIRTY_FROM_NAME(ANetShootGameState, bAllPlayersReady, this)
		bAllPlayersReady = bTempAllReady;
		AllReadyDelegate.Broadcast();
	}
}

void ANetShootGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;
	
	DOREPLIFETIME_WITH_PARAMS_FAST(ANetShootGameState, bAllPlayersReady, SharedParams);
}
