// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "BackpackItemInterface.generated.h"

USTRUCT(BlueprintType)
struct FBackpackItemInfo
{
	GENERATED_USTRUCT_BODY()

	//物品具体类型标识（唯一id），用于在Class的基础上细分类型，比如不同的枪械虽然类相同，但其代表的类型可能不同
	UPROPERTY(BlueprintReadWrite)
	FName ItemType;

	//物品的类型，用于反序列化
	UPROPERTY(BlueprintReadWrite)
	UClass* ItemClass;

	//物品的数量，可重叠物品可以有大于1的数量
	UPROPERTY(BlueprintReadWrite)
	int32 ItemNum;

	//物品是否不可堆叠
	UPROPERTY(BlueprintReadWrite)
	uint32 bIsUnique : 1;
	
	//物品的其他信息，用于反序列化还原物品真实状态
	UPROPERTY()
	TArray<uint8> ItemStatus;
	
	bool operator==(const FBackpackItemInfo& BackpackItemInfo) const
	{
		return ItemType == BackpackItemInfo.ItemType && !bIsUnique;
	}
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI, Blueprintable, Category="Backpack")
class UBackpackItemInterface : public UInterface
{
	GENERATED_BODY()

	
};

/**
 * 
 */
class NETSHOOTGAME_API IBackpackItemInterface
{
	GENERATED_BODY()
	
	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	//比较 Actor 和 Info 是否相同 (纯C++)
	virtual bool ActorEqualToInfo(AActor* Actor, FBackpackItemInfo& Info);
	
	//序列化，将对象自身序列化为 FBackpackItemInfo 类型
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Backpack")
	FBackpackItemInfo GenerateBackpackItemInfo(AActor* Actor);
	virtual FBackpackItemInfo GenerateBackpackItemInfo_Implementation(AActor* Actor);
	
	//反序列化，根据 FBackpackItemInfo 的信息生成一个新的对象
	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category="Backpack")
	void InitItemBeforeSpawn(FBackpackItemInfo& Info, AActor* Actor);
	virtual void InitItemBeforeSpawn_Implementation(FBackpackItemInfo& Info, AActor* Actor);
};
