// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SkeletalMeshComponent.h"
#include "GrenadeComponent.generated.h"

#define SELF_DEFINED_EXPLODE_TRACE_CHANNEL TraceTypeQuery3
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FExplodeFinishDelegate);

/**
 * 
 */
UCLASS(Blueprintable, ClassGroup=(Rendering, Common), hidecategories=Object, config=Engine, editinlinenew, meta=(BlueprintSpawnableComponent))
class NETSHOOTGAME_API UGrenadeComponent : public USkeletalMeshComponent
{
	GENERATED_BODY()
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Grenade, meta = (AllowPrivateAccess = "true"))
	class UPointLightComponent* SignalLight;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Grenade, meta = (AllowPrivateAccess = "true"))
	class URadialForceComponent* RadialForce;
	
	UPROPERTY(ReplicatedUsing=Rep_OnExplodeStart)
	uint32 bIsStart : 1;
	
public:

	UGrenadeComponent();

	UPROPERTY(EditDefaultsOnly)
	class UNiagaraSystem* ExplodeNiagaraSystem;

	UFUNCTION(NetMulticast, Reliable)
	void PlayExplodeNiagaraAnimation();
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Grenade)
	float Damage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Grenade)
	float DamageReduction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Grenade)
	float DamageRadius;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Grenade)
	float ExplodeDelayTime;
	
	UFUNCTION(BlueprintCallable, Category = Grenade)
	void StartExplode();
	
	FTimerHandle ExplodeDelayTimer;

	FExplodeFinishDelegate ExplodeFinishDelegate;

private:

	UFUNCTION()
	void Rep_OnExplodeStart();
	
	void Explode();
	
protected:
	
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;
};
