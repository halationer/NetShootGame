// Fill out your copyright notice in the Description page of Project Settings.


#include "BackpackItemInterface.h"

#include "Kismet/GameplayStatics.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

// Add default functionality here for any IBackpackItemInterface functions that are not pure virtual.
bool IBackpackItemInterface::ActorEqualToInfo(AActor* Actor, FBackpackItemInfo& Info)
{
	GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5.0, FColor::Orange, Actor->GetClass()->GetName());
	return Actor->GetClass() == Info.ItemClass;
}

FBackpackItemInfo IBackpackItemInterface::GenerateBackpackItemInfo_Implementation(AActor* Actor)
{
	FBackpackItemInfo BackpackItemInfo;
	BackpackItemInfo.ItemClass = Actor->GetClass();
	BackpackItemInfo.ItemNum = 1;
	BackpackItemInfo.bIsUnique = false;
	return BackpackItemInfo;
}

void IBackpackItemInterface::InitItemBeforeSpawn_Implementation(FBackpackItemInfo& Info, AActor* Actor)
{
	
}