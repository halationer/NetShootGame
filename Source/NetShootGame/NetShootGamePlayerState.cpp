// Fill out your copyright notice in the Description page of Project Settings.


#include "NetShootGamePlayerState.h"

#include "NetShootGameState.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

ANetShootGamePlayerState::ANetShootGamePlayerState()
{
	bIsReady = false;
}

void ANetShootGamePlayerState::BeginPlay()
{
	Super::BeginPlay();

	// palyer enter game should update the gamestate
    UWorld* World = GetWorld();
    if(World)
    {
        ANetShootGameState* GameState = World->GetGameState<ANetShootGameState>();
        if(GameState) GameState->UpdateAllPlayersReady();
    }
}

void ANetShootGamePlayerState::SetReady_Implementation(const bool IsReady)
{
	// Use push model to jump rep check, in order to speed up replicating.
	MARK_PROPERTY_DIRTY_FROM_NAME(ANetShootGamePlayerState, bIsReady, this);
	bIsReady = IsReady;
	
	UWorld* World = GetWorld();
	check(World)

	ANetShootGameState* GameState = World->GetGameState<ANetShootGameState>();
	check(GameState)
	GameState->UpdateAllPlayersReady();
}

void ANetShootGamePlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;
	
	DOREPLIFETIME_WITH_PARAMS_FAST(ANetShootGamePlayerState, bIsReady, SharedParams);
}

void ANetShootGamePlayerState::OnReadyChange()
{
	ReadyChangeDelegate.Broadcast();
}
