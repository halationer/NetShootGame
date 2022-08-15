// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetShootGameGameMode.h"

#include "EngineUtils.h"
#include "NetShootGameCharacter.h"
#include "NetShootGamePlayerController.h"
#include "NetShootGamePlayerState.h"
#include "NetShootGameSession.h"
#include "NetShootGameState.h"
#include "GameFramework/GameSession.h"
#include "Net/OnlineEngineInterface.h"
#include "UObject/ConstructorHelpers.h"

ANetShootGameGameMode::ANetShootGameGameMode()
{
	// set default pawn class to our Blueprinted character
	// static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	// if (PlayerPawnBPClass.Class != NULL)
	// {
	// 	DefaultPawnClass = PlayerPawnBPClass.Class;
	// }

	// Default Gameplay Classes Setting
	PlayerStateClass = ANetShootGamePlayerState::StaticClass();
	GameStateClass = ANetShootGameState::StaticClass();

	RoomMaxPlayerNum = 3;
}

void ANetShootGameGameMode::PostLogin(APlayerController* NewPlayer)
{
	// Server Allocate Pawns for Players
	if(GetLocalRole() == ROLE_Authority)
	{
		// Some Session Infomation
		// int32 CurrentPlayerNum = GetNumPlayers();
		// ANetShootGameSession* CurrentGameSession = Cast<ANetShootGameSession>(GameSession);
		// GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5.0f, FColor::Orange, FString::Printf(TEXT("Room %s after %d player login"), GetData(CurrentGameSession->RoomName), CurrentPlayerNum));
		
		
		// Find Player Start and Spawn Pawn for NewPlayer, in order to view Session Room UI
		// Fix the problem of -- when client exit room and get in again, the camera will in wrong state
		APlayerStart* FirstPlayerStart = GetPlayerStart(0);
		FVector StartLocation = FVector(0, 0, 0);
		FRotator StartRotation(ForceInit);
		
		if(FirstPlayerStart)
		{
			StartLocation = FirstPlayerStart->GetActorLocation();
			StartRotation.Yaw = FirstPlayerStart->GetActorRotation().Yaw;
		}
		else UE_LOG(LogTemp, Error, TEXT("Find no player start!"));
		
		FTransform Transform = FTransform(StartRotation, StartLocation);
		APawn* NewPawn = SpawnDefaultPawnAtTransform(NewPlayer, Transform);
		NewPlayer->Possess(NewPawn);

		
		// ANetShootGamePlayerController* PlayerController = Cast<ANetShootGamePlayerController>(NewPlayer);
	}
	
	Super::PostLogin(NewPlayer);

	if(GetLocalRole() == ROLE_Authority)
	{
		// Set Ready to true if Server Controller
		if(NewPlayer->IsLocalController())
		{
			UE_LOG(LogTemp, Warning, TEXT("Server Player set Ready true"));

			ANetShootGamePlayerState* PlayerState = NewPlayer->GetPlayerState<ANetShootGamePlayerState>();
			if(PlayerState)
			{
				PlayerState->SetReady(true);
			}
		}
	}
}

APlayerStart* ANetShootGameGameMode::GetPlayerStart(const int32 TargetIndex) const
{
	UWorld* World = GetWorld();

	if(World)
	{
		int32 CurrentIndex = 0;
		
		for(TActorIterator<APlayerStart> It(World); It; ++It)
		{
			UE_LOG(LogTemp, Warning, TEXT("Find %d player start"), CurrentIndex);
			if(CurrentIndex == TargetIndex) return *It;
			++CurrentIndex;
		}
	}

	return nullptr;
}

bool ANetShootGameGameMode::IsAllPlayersReady() const
{
	ANetShootGameState* NetShootGameState = GetGameState<ANetShootGameState>();
	if(NetShootGameState)
	{
		return NetShootGameState->GetAllPlayersReady();
	}
	return false;
}

void ANetShootGameGameMode::StartGame()
{
	if(!IsAllPlayersReady())
	{
		UE_LOG(LogTemp, Warning, TEXT("Not all players Ready!"));
	}
}
