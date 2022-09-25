// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GrenadeActor.h"
#include "ItemBaseActor.h"
#include "GameFramework/Character.h"
#include "NetShootGameCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLowLifeDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDeathDelegate);

UCLASS(config=Game)
class ANetShootGameCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* FollowCamera;

	/** Weapon */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Item, meta = (AllowPrivateAccess = "true"))
	class UWeaponBaseComponent* EquippedWeapon;

	/** Backpack */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Backpack, meta = (AllowPrivateAccess = "ture"))
	class UBackpackComponent* Backpack;
	
public:
	ANetShootGameCharacter();

	virtual void BeginPlay() override;

	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseTurnRate;

	/** Base look up/down rate, in deg/sec. Other scaling may affect final rate. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Camera)
	float BaseLookUpRate;


	// Aiming
	UFUNCTION(BlueprintCallable)
	void OnAimingStart();
	
	UFUNCTION(BlueprintCallable)
	void OnAimingEnd();
	
	UFUNCTION(Server, Reliable)
	void SetAiming_Server(bool bNewAiming);
	
	UFUNCTION(NetMulticast, Reliable)
	void SetAiming_Multicast(bool bNewAiming);
	
	UFUNCTION(BlueprintImplementableEvent)
	void SetAimingCameraOffset_BP(bool bNewAiming);

	
	// Sprint (speed up)
	UFUNCTION(BlueprintCallable)
	void OnSpeedUpStart();

	UFUNCTION(BlueprintCallable)
	void OnSpeedUpEnd();

	UFUNCTION(Server, Reliable)
	void SetSpeedUp_Server(bool bNewSpeedUp);

	UFUNCTION(NetMulticast, Reliable)
	void SetSpeedUp_Multicast(bool bNewSpeedUp);


	// PickUp
	UFUNCTION(BlueprintCallable)
	void PickUpSelectedWeapon();

	UFUNCTION(BlueprintCallable)
	void ThrowCurrentWeapon();

	UFUNCTION(Server, Reliable)
	void PickUp_Server();

	UFUNCTION(Server, Reliable)
	void Throw_Server();

	// Backpack Pick Up
	UFUNCTION(BlueprintCallable)
	void BackpackPickUp();

	UFUNCTION(BlueprintCallable)
	void BackpackThrow();
	
	UFUNCTION(Server, Reliable)
	void BackpackPickUp_Server();
	
	UFUNCTION(Server, Reliable)
	void BackpackThrow_Server();

	
	//Fire
	UFUNCTION(BlueprintCallable)
	void Fire();

	UFUNCTION(BlueprintCallable)
	void FireStop();
	
	UFUNCTION(Server, Reliable)
	void Fire_Server();
	
	UFUNCTION(Server, Reliable)
	void FireStop_Server();

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Fire")
	FVector2D FrontSightPositionOnScreen;

	UPROPERTY(Transient, BlueprintReadWrite)
	uint32 bHitPawn : 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category="Fire")
	float AutoAimStopDelay;
	
	FTimerHandle AutoAimStopTimer;

	// Grenade
	UFUNCTION(BlueprintCallable, Server, Reliable)
	void ThrowGrenadeStart();

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void ThrowGrenadeRelease();

	UFUNCTION(BlueprintCallable)
	void StartGrenade();

	UFUNCTION(BlueprintCallable)
	void ThrowOffGrenade();

	
	// Animation State Control Properties
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category="AnimState")
	uint32 bIsThrowingGrenade : 1;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category="AnimState")
    uint32 bIsCarryingWeapon : 1;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category="AnimState")
    uint32 bIsAiming : 1;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category="AnimState")
	float LookUpAngle;

	UPROPERTY(EditDefaultsOnly, Category="Movement")
	float MaxWalkSpeedNotAiming;
	
	UPROPERTY(EditDefaultsOnly, Category="Movement")
	float MaxWalkSpeedIsAiming;
	
	UPROPERTY(EditDefaultsOnly, Category="Movement")
	float MaxWalkSpeedSprint;
	
	UPROPERTY(EditDefaultsOnly, Category="Movement")
	float JumpZVelocityNotAiming;
	
	UPROPERTY(EditDefaultsOnly, Category="Movement")
	float JumpZVelocityIsAiming;

	
	// Pick Ups
	UPROPERTY(EditDefaultsOnly, Category="Item")
	FName WeaponSocketName;

	UPROPERTY(EditDefaultsOnly, Category="Item")
	FName GrenadeSocketName;

	UPROPERTY(EditDefaultsOnly, Category="Item")
	FName BackpackSocketName;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category="Item")
	int32 CurrentGrenadeNum;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Item")
	int32 MaxGrenadeNum;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Item")
	TArray<AItemBaseActor*> ItemsCanTouch;

	UPROPERTY(Transient, BlueprintReadOnly, Category="Item")
	AGrenadeActor* HandledGrenade;

	FORCEINLINE void CloseToItem(AItemBaseActor* Item) { ItemsCanTouch.AddUnique(Item); }
	FORCEINLINE void AwayFromItem(AItemBaseActor* Item) { ItemsCanTouch.Remove(Item); }

	
	// GamePlay
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category="Character")
	float MaxHealth;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, ReplicatedUsing=OnCurrentHealthChange_Rep, Category="Character")
	float CurrentHealth;

	UPROPERTY(BlueprintAssignable)
	FDeathDelegate DeathDelegate;
	
	FTimerHandle DestroyTimer;
	void DestroyFunc() { RespawnSelfClassCharacter(); Destroy(); }
	// the respawn function should expose to the blueprint, in order to override the spawn class
	UFUNCTION(BlueprintNativeEvent)
	void RespawnSelfClassCharacter();

	UFUNCTION(NetMulticast, Reliable)
	void DeathToBeRagDoll_MultiCast();
	UFUNCTION(BlueprintImplementableEvent)
	void RemoveWidgetsOfCharacter();
	
	UPROPERTY(BlueprintAssignable)
	FLowLifeDelegate LowLifeDelegate;

	UPROPERTY(EditDefaultsOnly, Category="Character")
	float LowLifeRatio;

	UFUNCTION(BlueprintCallable)
	float GetLifeRatio() const { return CurrentHealth / MaxHealth; }
	
	virtual float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	void ActorBeKilled(AController* KillerController);

	UFUNCTION()
	void OnCurrentHealthChange_Rep();

	
	// IK
	UPROPERTY(EditDefaultsOnly, Category="IK")
	FName RightFootSocket = TEXT("IK_RightFoot");

	UPROPERTY(EditDefaultsOnly, Category="IK")
	FName LeftFootSocket = TEXT("IK_LeftFoot");

	UPROPERTY(EditAnywhere, Category="IK")
	float IKInterpolationSpeed;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="IK")
	float IKLeftFootOffset;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="IK")
	float IKRightFootOffset;
	
private:

	float IKFootTrace(FName FootSocketName);
	
protected:

	virtual void Tick(float DeltaSeconds) override;
	
	/** Resets HMD orientation in VR. */
	void OnResetVR();

	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate	This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);

	UFUNCTION(BlueprintCallable, Server, Reliable)
	void SetLookUpAngle(float Angle);

	void LookUp(float Rate);
	
	
	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);
	
protected:
	// APawn interface
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	// End of APawn interface
	
public:
	/** Returns CameraBoom subobject **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
	/** Returns FollowCamera subobject **/
	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }
	
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;
	
};

