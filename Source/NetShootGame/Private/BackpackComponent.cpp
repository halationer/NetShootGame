// Fill out your copyright notice in the Description page of Project Settings.


#include "BackpackComponent.h"

#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

/** 将物体转换为 BackpackItemInfo 形式并记录在背包中
 * @return bool 如果物体实现了 IBackpackItemInterface 接口，则返回 true，否则 false 并且拒绝放入背包
 * @param Actor 将要放入背包的物体
 */
bool UBackpackComponent::AddToBag(AActor* Actor)
{
	IBackpackItemInterface* Item = Cast<IBackpackItemInterface>(Actor);

	FBackpackItemInfo BackpackItemInfo;
	if(Item)
	{
		// 对于蓝图可调用接口，必须使用 Execute_ 方式调用
		// BackpackItemInfo = Item->GenerateBackpackItemInfo(Actor);
		BackpackItemInfo = Item->Execute_GenerateBackpackItemInfo(Actor, Actor);
		
		//与AddUnique的逻辑相似
		int32 Index;
		if (BagContainer.Find(BackpackItemInfo, Index))
		{
			++BagContainer[Index].ItemNum;
		}
		else
		{
			BagContainer.Push(BackpackItemInfo);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("The actor class %s does not implement IBackpackItemInterface."), GetData(Actor->GetClass()->GetName()));
	}

	return Item != nullptr;
}

void UBackpackComponent::RemoveFromBag(int32 Index, int32 Num)
{
	FBackpackItemInfo& IndexItem = BagContainer[Index];

	if(IndexItem.ItemNum > 0)
	{
		SpawnActorByInfo(IndexItem, this->GetComponentTransform());
	}
	
	if(IndexItem.ItemNum <= Num)
	{
		BagContainer.RemoveAt(Index);
	}
	else
	{
		IndexItem.ItemNum -= Num;
	}
}

void UBackpackComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UBackpackComponent, BagContainer);
}

AActor* UBackpackComponent::SpawnActorByInfo(FBackpackItemInfo& Info, const FTransform& Transform) const
{
	AActor* Actor = UGameplayStatics::BeginDeferredActorSpawnFromClass(this->GetOwner(), Info.ItemClass, Transform);
	
	IBackpackItemInterface* Item = Cast<IBackpackItemInterface>(Actor);
	if(Item)
	{
		Item->Execute_InitItemBeforeSpawn(Actor, Info, Actor);
	}
	
	UGameplayStatics::FinishSpawningActor(Actor, Transform);
	
	return Actor;
}
