// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "WeaponBaseComponent.generated.h"

USTRUCT(BlueprintType)
struct FWeaponAttribute
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite)
	int32 Id;
	
	UPROPERTY(BlueprintReadWrite)
	FString Name;
	
	UPROPERTY(BlueprintReadWrite)
	USkeletalMesh* SkeletonMesh;
	
	UPROPERTY(BlueprintReadWrite)
	UStaticMesh* BulletMesh;

	// NONE for clear attribute of a component
	static FWeaponAttribute NONE;
};

/**
 * 
 */
UCLASS(Blueprintable, ClassGroup=(Rendering, Common), hidecategories=Object, config=Engine, editinlinenew, meta=(BlueprintSpawnableComponent))
class NETSHOOTGAME_API UWeaponBaseComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()

protected:
	UPROPERTY(ReplicatedUsing=OnAttributeChange)
	FWeaponAttribute WeaponAttribute;
	
public:

	UFUNCTION(BlueprintCallable)
	FORCEINLINE FWeaponAttribute GetAttribute() const { return WeaponAttribute; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE USkeletalMesh* GetSkeletalMesh() const { return WeaponAttribute.SkeletonMesh; }

	UFUNCTION(BlueprintCallable)
	void ClearWeaponAttribute() { SetWeaponAttribute(FWeaponAttribute::NONE); }
	
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void SetWeaponAttribute(const FWeaponAttribute& NewAttribute);
	
	UFUNCTION(BlueprintCallable)
	void OnAttributeChange();

	UFUNCTION(BlueprintCallable)
	void Fire(const FTransform& MuzzleTransform, float TraceDistance);
	
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;
};
