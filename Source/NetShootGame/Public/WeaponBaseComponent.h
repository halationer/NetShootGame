// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/DataTable.h"
#include "WeaponBaseComponent.generated.h"

USTRUCT(BlueprintType)
struct FWeaponAttribute : public FTableRowBase
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

	UPROPERTY(BlueprintReadWrite)
	UTexture2D* DisplayImage;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFireOnceDelegate);

/**
 * 
 */
UCLASS(Blueprintable, ClassGroup=(Rendering, Common), hidecategories=Object, config=Engine, editinlinenew, meta=(BlueprintSpawnableComponent))
class NETSHOOTGAME_API UWeaponBaseComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Fire, meta = (AllowPrivateAccess = "true"))
	class USpotLightComponent* FireLight;

protected:
	UPROPERTY(ReplicatedUsing=OnAttributeChange)
	FWeaponAttribute WeaponAttribute;
	
	
public:
	UWeaponBaseComponent();
	
	UFUNCTION(BlueprintCallable)
	FORCEINLINE FWeaponAttribute GetAttribute() const { return WeaponAttribute; }

	UFUNCTION(BlueprintCallable)
	FORCEINLINE USkeletalMesh* GetSkeletalMesh() const { return WeaponAttribute.SkeletonMesh; }

	UFUNCTION(BlueprintCallable)
	void ClearWeaponAttribute() { SetWeaponAttribute(FWeaponAttribute()); }
	
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void SetWeaponAttribute(const FWeaponAttribute& NewAttribute);
	
	UFUNCTION(BlueprintCallable)
	void OnAttributeChange();

	UFUNCTION(BlueprintCallable, Category="Fire")
	void FireStart();

	UFUNCTION(BlueprintCallable, Category="Fire")
	void FireStop();
	
	UPROPERTY(BlueprintAssignable)
	FFireOnceDelegate FireOnceDelegate;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFireOnce();
	
	virtual FTransform GetFireStartTransform() const;
	
	UFUNCTION(BlueprintCallable, Server, Unreliable)
	void UpdateTargetLocation(const FVector& NewTargetLocation);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void SetWithTarget(const bool NewWithTarget);
	
	UPROPERTY(EditDefaultsOnly)
	class UNiagaraSystem* FireNiagaraSystem;
	
protected:

	// Only read by server to correct the fire direction
	UPROPERTY(Transient)
	FVector TargetLocation;

	// Only read by server to correct the fire direction
	UPROPERTY(Transient)
	uint32 bWithTarget : 1;
	
	void FireOnce();
	
	void AutoFireStart();

	void AutoFireStop();

	void UnlockFire() { bIsFire = false; }

public:
	
	UPROPERTY(Transient, VisibleAnywhere, BlueprintReadOnly, Replicated, Category="Fire")
	uint32 bIsFire : 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fire")
	uint32 bIsAuto : 1;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fire")
	float FireRate;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fire")
	float FireDistance;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Fire")
	float FireBaseDamage;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category="Fire")
	FName FireSocketName;
	
	FTimerHandle AutoFireTimerHandle;
	FTimerHandle FireOnceTimerHandle;
	
	// Replicate
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;
};
