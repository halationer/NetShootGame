// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ItemBaseActor.h"
#include "GameFramework/Character.h"
#include "NetShootGameCharacter.generated.h"

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
	UFUNCTION()
	void OnAimingStart();
	
	UFUNCTION()
	void OnAimingEnd();
	
	UFUNCTION(Server, Reliable)
	void SetAiming_Server(bool bNewAiming);
	
	UFUNCTION(NetMulticast, Reliable)
	void SetAiming_Multicast(bool bNewAiming);
	
	UFUNCTION(BlueprintImplementableEvent)
	void SetAimingCameraOffset_BP(bool bNewAiming);

	// Sprint (speed up)
	UFUNCTION()
	void OnSpeedUpStart();

	UFUNCTION()
	void OnSpeedUpEnd();

	UFUNCTION(Server, Reliable)
	void SetSpeedUp_Server(bool bNewSpeedUp);

	UFUNCTION(NetMulticast, Reliable)
	void SetSpeedUp_Multicast(bool bNewSpeedUp);


	// PickUp
	UFUNCTION()
	void PickUpSelectedWeapon();

	UFUNCTION()
	void ThrowCurrentWeapon();

	UFUNCTION(Server, Reliable)
	void PickUp_Server();

	UFUNCTION(Server, Reliable)
	void Throw_Server();

	
	// Animation State Control Properties

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category="AnimState")
	uint32 bIsThrowingGrenade : 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category="AnimState")
	uint32 bIsFire : 1;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category="AnimState")
    uint32 bIsCarryingWeapon : 1;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category="AnimState")
    uint32 bIsAiming : 1;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category="AnimState")
	float LookUpAngle;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category="AnimState")
	float IKLeftFootOffset;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Replicated, Category="AnimState")
	float IKRightFootOffset;

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
	
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Item")
	TArray<AItemBaseActor*> ItemsCanTouch;

	FORCEINLINE void CloseToItem(AItemBaseActor* Item) { ItemsCanTouch.AddUnique(Item); }
	FORCEINLINE void AwayFromItem(AItemBaseActor* Item) { ItemsCanTouch.Remove(Item); }
	
protected:

	UFUNCTION(Server, Reliable)
	void Fire();
	
	UFUNCTION(Server, Reliable)
	void FireStop();
	
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

	UFUNCTION(Server, Reliable)
	void SetLookUpAngle(float Angle);

	void LookUp(float Rate);
	
	
	/** Handler for when a touch input begins. */
	void TouchStarted(ETouchIndex::Type FingerIndex, FVector Location);

	/** Handler for when a touch input stops. */
	void TouchStopped(ETouchIndex::Type FingerIndex, FVector Location);
	
	UFUNCTION(Server, Reliable)
	void ThrowGrenadeStart();

	UFUNCTION(Server, Reliable)
	void ThrowGrenadeRelease();
	
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

