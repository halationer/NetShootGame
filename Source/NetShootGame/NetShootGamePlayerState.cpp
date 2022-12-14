// Fill out your copyright notice in the Description page of Project Settings.


#include "NetShootGamePlayerState.h"

#include "NetShootGameState.h"
#include "Net/UnrealNetwork.h"
#include "Net/Core/PushModel/PushModel.h"

int32 ANetShootGamePlayerState::player_name_id = 0;

ANetShootGamePlayerState::ANetShootGamePlayerState()
{
	bIsReady = false;

	DestroyTargetNum = 0;
	KillNum = 0;
	DeathNum = 0;

	FString DefaultName = FString::Printf(TEXT("player_%d"), ANetShootGamePlayerState::player_name_id++);
	PlayerName = DefaultName;
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
	
	DOREPLIFETIME_WITH_PARAMS_FAST(ANetShootGamePlayerState, bIsReady, SharedParams)

	DOREPLIFETIME(ANetShootGamePlayerState, DestroyTargetNum)
	DOREPLIFETIME(ANetShootGamePlayerState, KillNum)
	DOREPLIFETIME(ANetShootGamePlayerState, DeathNum)
	DOREPLIFETIME(ANetShootGamePlayerState, PlayerName)
}

void ANetShootGamePlayerState::Rep_OnReadyChange()
{
	ReadyChangeDelegate.Broadcast();
}
