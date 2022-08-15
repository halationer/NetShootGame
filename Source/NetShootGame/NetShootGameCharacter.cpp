// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetShootGameCharacter.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "WeaponBaseActor.h"
#include "WeaponBaseComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Net/UnrealNetwork.h"

//////////////////////////////////////////////////////////////////////////
// ANetShootGameCharacter

ANetShootGameCharacter::ANetShootGameCharacter()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;
	LookUpAngle = 0.f;
	MaxWalkSpeedSprint = 800.f;
	MaxWalkSpeedNotAiming = 600.f;
	MaxWalkSpeedIsAiming = 200.f;
	JumpZVelocityNotAiming = 400.f;
	JumpZVelocityIsAiming = 200.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f); // ...at this rotation rate
	GetCharacterMovement()->JumpZVelocity = 600.f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	// Create an empty Weapon
	EquippedWeapon = CreateDefaultSubobject<UWeaponBaseComponent>(TEXT("EquippedWeapon"));
	EquippedWeapon->SetupAttachment(GetMesh());
	EquippedWeapon->SetIsReplicated(true);
	EquippedWeapon->SetActive(false);
	WeaponSocketName = TEXT("Weapon");
	
	// Note: The skeletal mesh and anim blueprint references on the Mesh component (inherited from Character) 
	// are set in the derived blueprint asset named MyCharacter (to avoid direct content references in C++)
}

void ANetShootGameCharacter::BeginPlay()
{
	Super::BeginPlay();
	EquippedWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules(EAttachmentRule::KeepRelative, true), WeaponSocketName);
}

//////////////////////////////////////////////////////////////////////////
// Input

void ANetShootGameCharacter::SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent)
{
	// Set up gameplay key bindings
	check(PlayerInputComponent);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &ACharacter::Jump);
	PlayerInputComponent->BindAction("Jump", IE_Released, this, &ACharacter::StopJumping);

	PlayerInputComponent->BindAxis("MoveForward", this, &ANetShootGameCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ANetShootGameCharacter::MoveRight);

	// We have 2 versions of the rotation bindings to handle different kinds of devices differently
	// "turn" handles devices that provide an absolute delta, such as a mouse.
	// "turnrate" is for devices that we choose to treat as a rate of change, such as an analog joystick
	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("TurnRate", this, &ANetShootGameCharacter::TurnAtRate);
	PlayerInputComponent->BindAxis("LookUp", this, &ANetShootGameCharacter::LookUp);
	PlayerInputComponent->BindAxis("LookUpRate", this, &ANetShootGameCharacter::LookUpAtRate);

	// handle touch devices
	PlayerInputComponent->BindTouch(IE_Pressed, this, &ANetShootGameCharacter::TouchStarted);
	PlayerInputComponent->BindTouch(IE_Released, this, &ANetShootGameCharacter::TouchStopped);

	// VR headset functionality
	PlayerInputComponent->BindAction("ResetVR", IE_Pressed, this, &ANetShootGameCharacter::OnResetVR);
	
	// Aiming Mode
	PlayerInputComponent->BindAction("Aim", IE_Pressed, this, &ANetShootGameCharacter::OnAimingStart);
	PlayerInputComponent->BindAction("Aim", IE_Released, this, &ANetShootGameCharacter::OnAimingEnd);
	
	PlayerInputComponent->BindAction("SpeedUp", IE_Pressed, this, &ANetShootGameCharacter::OnSpeedUpStart);
	PlayerInputComponent->BindAction("SpeedUp", IE_Released, this, &ANetShootGameCharacter::OnSpeedUpEnd);
	
	PlayerInputComponent->BindAction("PickUpWeapon", IE_Pressed, this, &ANetShootGameCharacter::PickUpSelectedWeapon);
	PlayerInputComponent->BindAction("ThrowWeapon", IE_Pressed, this, &ANetShootGameCharacter::ThrowCurrentWeapon);

	PlayerInputComponent->BindAction("ThrowGrenade", IE_Pressed, this, &ANetShootGameCharacter::ThrowGrenadeStart);
	PlayerInputComponent->BindAction("ThrowGrenade", IE_Released, this, &ANetShootGameCharacter::ThrowGrenadeRelease);

	PlayerInputComponent->BindAction("Fire", IE_Pressed, this, &ANetShootGameCharacter::Fire);
	PlayerInputComponent->BindAction("Fire", IE_Released, this, &ANetShootGameCharacter::FireStop);
}

void ANetShootGameCharacter::Fire_Implementation()
{
	bIsFire = true;
}

void ANetShootGameCharacter::FireStop_Implementation()
{
	bIsFire = false;
}

void ANetShootGameCharacter::OnResetVR()
{
	// If NetShootGame is added to a project via 'Add Feature' in the Unreal Editor the dependency on HeadMountedDisplay in NetShootGame.Build.cs is not automatically propagated
	// and a linker error will result.
	// You will need to either:
	//		Add "HeadMountedDisplay" to [YourProject].Build.cs PublicDependencyModuleNames in order to build successfully (appropriate if supporting VR).
	// or:
	//		Comment or delete the call to ResetOrientationAndPosition below (appropriate if not supporting VR)
	UHeadMountedDisplayFunctionLibrary::ResetOrientationAndPosition();
}

void ANetShootGameCharacter::ThrowGrenadeStart_Implementation()
{
	bIsThrowingGrenade = true;
}

void ANetShootGameCharacter::ThrowGrenadeRelease_Implementation()
{
	bIsThrowingGrenade = false;
}

void ANetShootGameCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
		Jump();
}

void ANetShootGameCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
		StopJumping();
}

void ANetShootGameCharacter::TurnAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void ANetShootGameCharacter::SetLookUpAngle_Implementation(float Angle)
{
	LookUpAngle = Angle;
}

void ANetShootGameCharacter::LookUp(float Rate)
{
	AddControllerPitchInput(Rate);
	SetLookUpAngle(GetControlRotation().Pitch);
}

void ANetShootGameCharacter::LookUpAtRate(float Rate)
{
	// calculate delta for this frame from the rate information
	AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
	SetLookUpAngle(GetControlRotation().Pitch);
}

void ANetShootGameCharacter::MoveForward(float Value)
{
	if ((Controller != nullptr) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ANetShootGameCharacter::MoveRight(float Value)
{
	if ( (Controller != nullptr) && (Value != 0.0f) )
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);
	
		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}


///// Functions about Aiming /////

void ANetShootGameCharacter::OnAimingStart()
{
	SetAiming_Server(true);
	SetAimingCameraOffset_BP(true);
}

void ANetShootGameCharacter::OnAimingEnd()
{
	SetAiming_Server(false);
	SetAimingCameraOffset_BP(false);
}

void ANetShootGameCharacter::SetAiming_Server_Implementation(bool bNewAiming)
{
	bIsAiming = bNewAiming;
	SetAiming_Multicast(bNewAiming);
}

void ANetShootGameCharacter::SetAiming_Multicast_Implementation(bool bNewAiming)
{
	if(bNewAiming)
	{
		bUseControllerRotationYaw = true;

		UCharacterMovementComponent* Movement = GetCharacterMovement();
		if(Movement)
		{
			Movement->bOrientRotationToMovement = false;
			Movement->MaxWalkSpeed = MaxWalkSpeedIsAiming;
			Movement->JumpZVelocity = JumpZVelocityIsAiming;
		}
	}
	else
	{
		bUseControllerRotationYaw = false;
		
		UCharacterMovementComponent* Movement = GetCharacterMovement();
		if(Movement)
		{
			Movement->bOrientRotationToMovement = true;
			Movement->MaxWalkSpeed = MaxWalkSpeedNotAiming;
			Movement->JumpZVelocity = JumpZVelocityNotAiming;
		}
	}
}


///// Functions about Sprint (Speed up) /////

void ANetShootGameCharacter::OnSpeedUpStart()
{
	SetSpeedUp_Server(true);
}

void ANetShootGameCharacter::OnSpeedUpEnd()
{
	SetSpeedUp_Server(false);
}

void ANetShootGameCharacter::SetSpeedUp_Server_Implementation(bool bNewSpeedUp)
{
	// Only Can Sprint When Empty Hand
	if(!bIsCarryingWeapon)
	{
		SetSpeedUp_Multicast(bNewSpeedUp);
	}
}

void ANetShootGameCharacter::SetSpeedUp_Multicast_Implementation(bool bNewSpeedUp)
{
	if(bNewSpeedUp)
	{
		UCharacterMovementComponent* Movement = GetCharacterMovement();
		if(Movement)
		{
			Movement->MaxWalkSpeed = MaxWalkSpeedSprint;
		}
	}
	else
	{
		UCharacterMovementComponent* Movement = GetCharacterMovement();
		if(Movement)
		{
			Movement->MaxWalkSpeed = MaxWalkSpeedNotAiming;
		}
	}
}

///// Character and Items /////

void ANetShootGameCharacter::PickUpSelectedWeapon()
{
	PickUp_Server();
}

void ANetShootGameCharacter::ThrowCurrentWeapon()
{
	Throw_Server();
}

void ANetShootGameCharacter::PickUp_Server_Implementation()
{
	if(ItemsCanTouch.Num() > 0)
	{
		AItemBaseActor* FirstItem = ItemsCanTouch.Top();
    
		if(IsValid(FirstItem))
		{
			ItemsCanTouch.Remove(FirstItem);
        	
			AWeaponBaseActor* FirstWeapon = Cast<AWeaponBaseActor>(FirstItem);
			if(FirstWeapon)
			{
				UWeaponBaseComponent* WeaponComponent = FirstWeapon->WeaponMesh;
				if(WeaponComponent)
				{
					if(bIsCarryingWeapon)
					{
						ThrowCurrentWeapon();
					}
					EquippedWeapon->SetWeaponAttribute(WeaponComponent->GetAttribute());
					EquippedWeapon->SetActive(true);
					bIsCarryingWeapon = true;
				}
			}
        	
			FirstItem->Destroy();
		}
	}
}

void ANetShootGameCharacter::Throw_Server_Implementation()
{
	
	UWorld* const World = GetWorld();
	if (World)
	{
		FTransform SpawnTransform = GetMesh()->GetSocketTransform(WeaponSocketName);
		AWeaponBaseActor* const SpawnWeapon = World->SpawnActor<AWeaponBaseActor>(AWeaponBaseActor::StaticClass(), SpawnTransform);
		
		SpawnWeapon->WeaponMesh->SetWeaponAttribute(EquippedWeapon->GetAttribute());

		FVector ThrowImpulse = GetActorForwardVector();
		ThrowImpulse = ThrowImpulse + FVector(0.f, 0.f, 0.2f);
		ThrowImpulse = 800.0f * ThrowImpulse;
		ThrowImpulse = ThrowImpulse + GetMovementComponent()->Velocity;
		SpawnWeapon->WeaponMesh->AddImpulse(ThrowImpulse);
	}
	
	EquippedWeapon->ClearWeaponAttribute();
	EquippedWeapon->SetActive(false);
	bIsCarryingWeapon = false;
}

///// Properties Replicate Config /////

void ANetShootGameCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANetShootGameCharacter, bIsThrowingGrenade);
	DOREPLIFETIME(ANetShootGameCharacter, bIsFire);
	DOREPLIFETIME(ANetShootGameCharacter, bIsCarryingWeapon);
	DOREPLIFETIME(ANetShootGameCharacter, bIsAiming);
	
	DOREPLIFETIME(ANetShootGameCharacter, LookUpAngle);
	
	DOREPLIFETIME(ANetShootGameCharacter, IKLeftFootOffset);
	DOREPLIFETIME(ANetShootGameCharacter, IKRightFootOffset);
}
