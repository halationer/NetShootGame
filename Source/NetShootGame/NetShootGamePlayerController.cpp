// Fill out your copyright notice in the Description page of Project Settings.


#include "NetShootGamePlayerController.h"

void ANetShootGamePlayerController::BindShowPlayerState_Client_Implementation()
{
	InputComponent->BindAction("ShowPlayerState", IE_Pressed, this, &ANetShootGamePlayerController::ShowPlayerState);
}

void ANetShootGamePlayerController::UnBindShowPlayerState_Client_Implementation()
{
	InputComponent->RemoveActionBinding("ShowPlayerState", IE_Pressed);
}
