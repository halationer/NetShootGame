// Copyright Epic Games, Inc. All Rights Reserved.

#include "NetShootGameCharacter.h"

#include "GrenadeActor.h"
#include "GrenadeComponent.h"
#include "HeadMountedDisplayFunctionLibrary.h"
#include "NetShootGamePlayerState.h"
#include "NetShootGameState.h"
#include "WeaponBaseActor.h"
#include "WeaponBaseComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/PlayerStart.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Net/UnrealNetwork.h"

//////////////////////////////////////////////////////////////////////////
// ANetShootGameCharacter

ANetShootGameCharacter::ANetShootGameCharacter()
{
	// Set size for collision and transform for character
	GetCapsuleComponent()->InitCapsuleSize(42.f, 90.0f);
	GetMesh()->SetRelativeTransform(FTransform(FRotator(0.f, -90.f, 0.f), FVector(0.f, 0.f, -90.f)));

	// set our turn rates for input
	BaseTurnRate = 45.f;
	BaseLookUpRate = 45.f;
	LookUpAngle = 0.f;
	MaxWalkSpeedSprint = 800.f;
	MaxWalkSpeedNotAiming = 600.f;
	MaxWalkSpeedIsAiming = 200.f;
	JumpZVelocityNotAiming = 400.f;
	JumpZVelocityIsAiming = 200.f;

	// game play
	MaxHealth = 100.0f;
	CurrentHealth = MaxHealth;
	LowLifeRatio = 0.3f;

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
	EquippedWeapon->SetVisibility(false);
	WeaponSocketName = TEXT("Weapon");

	// Fire
	FrontSightPositionOnScreen = FVector2D(0.5, 0.47);
	AutoAimStopDelay = 0.5f;

	//Grenade
	GrenadeSocketName = TEXT("Weapon");
	CurrentGrenadeNum = 1;
	MaxGrenadeNum = 3;

	//Mesh
	GetMesh()->SetSkeletalMesh(Cast<USkeletalMesh>(StaticLoadObject(USkeletalMesh::StaticClass(), nullptr, TEXT("SkeletalMesh'/Game/Characters/assests/Breathing_Idle.Breathing_Idle'"))));
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

void ANetShootGameCharacter::Fire()
{
	GetWorldTimerManager().ClearTimer(AutoAimStopTimer);
	OnAimingStart();
	Fire_Server();
}

void ANetShootGameCharacter::FireStop()
{
	GetWorldTimerManager().ClearTimer(AutoAimStopTimer);
	GetWorldTimerManager().SetTimer(AutoAimStopTimer, this, &ANetShootGameCharacter::OnAimingEnd, AutoAimStopDelay);
	FireStop_Server();
}

void ANetShootGameCharacter::Fire_Server_Implementation()
{
	if(bIsCarryingWeapon)
	{
		EquippedWeapon->FireStart();
	}
}

void ANetShootGameCharacter::FireStop_Server_Implementation()
{
	if(bIsCarryingWeapon)
	{
		EquippedWeapon->FireStop();
	}
}

void ANetShootGameCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// Update the weapon fix target direction, via pre ray cast
	if(IsLocallyControlled() && IsPlayerControlled() && bIsCarryingWeapon)
	{
		UWorld* World = GetWorld();
		if(World)
		{
			UGameViewportClient* Viewport = World->GetGameViewport();
			if(Viewport)
			{
				FVector2D ViewPortSize;
				Viewport->GetViewportSize(ViewPortSize);

				FVector StartLocation, TargetDirection;
				FVector2D ViewPortPose(ViewPortSize * FrontSightPositionOnScreen);
				UGameplayStatics::DeprojectScreenToWorld(Cast<APlayerController>(GetController()), ViewPortPose, StartLocation, TargetDirection);

				FHitResult Hit;
				FVector EndLocation = StartLocation + EquippedWeapon->FireDistance * TargetDirection;
				const FCollisionQueryParams QueryParams(TEXT("WeaponFire"), false, this);
				bool bHitSuccess = World->LineTraceSingleByChannel(Hit, StartLocation, EndLocation, ECC_Pawn, QueryParams);

				bHitPawn = bHitSuccess && Hit.Actor.IsValid() && Cast<APawn>(Hit.Actor) != nullptr;
				
				FVector TargetLocation = bHitSuccess ? Hit.Location : EndLocation;
				EquippedWeapon->UpdateTargetLocation(TargetLocation);

				// UE_LOG(LogTemp, Warning, TEXT("update location = %f, %f, %f"), TargetLocation.X, TargetLocation.Y, TargetLocation.Z);
			}
		}
	}

	// IK
	float RightTraceDistance = IKFootTrace(RightFootSocket);
	float LeftTraceDistance = IKFootTrace(LeftFootSocket);
	IKRightFootOffset = FMath::FInterpTo(IKRightFootOffset, RightTraceDistance, DeltaSeconds, IKInterpolationSpeed);
	IKLeftFootOffset = FMath::FInterpTo(IKLeftFootOffset, LeftTraceDistance, DeltaSeconds, IKInterpolationSpeed);
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
	UWorld* World = GetWorld();
	if(World && CurrentGrenadeNum > 0 && !bIsThrowingGrenade)
	{
		FTransform SpawnTransform = GetMesh()->GetSocketTransform(GrenadeSocketName);

		// Disable pickup sense, disable physics, attach to socket before spawn 
		AGrenadeActor* SpawnGrenade = Cast<AGrenadeActor>(UGameplayStatics::BeginDeferredActorSpawnFromClass(this, AGrenadeActor::StaticClass(), SpawnTransform));
		SpawnGrenade->InitCannotSensed();
		SpawnGrenade->GetGrenadeMesh()->SetSimulatePhysics(false);
		SpawnGrenade->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepRelativeTransform, GrenadeSocketName);
		SpawnGrenade->SetOwner(this);
		UGameplayStatics::FinishSpawningActor(SpawnGrenade, FTransform::Identity);
		
		HandledGrenade = SpawnGrenade;
		
		bIsThrowingGrenade = true;
	}

}

void ANetShootGameCharacter::ThrowGrenadeRelease_Implementation()
{
	bIsThrowingGrenade = false;
}

void ANetShootGameCharacter::StartGrenade()
{
	if(HandledGrenade && IsValid(HandledGrenade))
	{
		UGrenadeComponent* Grenade = HandledGrenade->GetGrenadeMesh();
		Grenade->StartExplode();
		--CurrentGrenadeNum;
	}
}

void ANetShootGameCharacter::ThrowOffGrenade()
{
	if(HandledGrenade && IsValid(HandledGrenade))
	{
		// detach from character hand
		HandledGrenade->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);

		FVector Velocity = GetMesh()->GetPhysicsLinearVelocityAtPoint(GetMesh()->GetSocketLocation(GrenadeSocketName), GrenadeSocketName);

		// FVector FixedDirection = GetActorRotation().Vector();
		// if(FixedDirection.Size() > 1.0f)
		// {
		// 	FixedDirection.Normalize();
		// 	FVector FixedVelocity = FVector::DotProduct(Velocity ,FixedDirection) * FixedDirection;
		// 	Velocity = FVector(FixedVelocity.X, FixedVelocity.Y, Velocity.Z);
		// }
		
		UGrenadeComponent* Grenade = HandledGrenade->GetGrenadeMesh();
		Grenade->SetSimulatePhysics(true);
		// FVector ThrowImpulse = GetActorForwardVector();
		// ThrowImpulse = ThrowImpulse + FVector(0.f, 0.f, 1.f);
		FVector ThrowImpulse = Velocity;
		ThrowImpulse = 100.0f * ThrowImpulse;
		ThrowImpulse = ThrowImpulse + GetMovementComponent()->Velocity;
		Grenade->AddImpulse(ThrowImpulse);
	}
}

void ANetShootGameCharacter::TouchStarted(ETouchIndex::Type FingerIndex, FVector Location)
{
	// Jump();
	// GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Cyan, FString::Printf(TEXT("TouchStart: %f, %f, %f"), Location.X, Location.Y, Location.Z));
}

void ANetShootGameCharacter::TouchStopped(ETouchIndex::Type FingerIndex, FVector Location)
{
	// StopJumping();
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
	if(bIsCarryingWeapon && !bIsAiming)
	{
		SetAiming_Server(true);
		SetAimingCameraOffset_BP(true);
	}
}

void ANetShootGameCharacter::OnAimingEnd()
{
	if(bIsAiming)
	{
		SetAiming_Server(false);
		SetAimingCameraOffset_BP(false);
	}
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
	OnSpeedUpEnd();
	PickUp_Server();
}

void ANetShootGameCharacter::ThrowCurrentWeapon()
{	
	if(bIsAiming)
	{
		OnAimingEnd();
	}
	
	Throw_Server();
}

void ANetShootGameCharacter::PickUp_Server_Implementation()
{
	if(ItemsCanTouch.Num() > 0)
	{
		AItemBaseActor* FirstItem = ItemsCanTouch.Top();
    
		if(IsValid(FirstItem) && FirstItem->CanPickedUp())
		{
			ItemsCanTouch.Remove(FirstItem);

			AWeaponBaseActor* FirstWeapon = Cast<AWeaponBaseActor>(FirstItem);
			AGrenadeActor* FirstGrenade = Cast<AGrenadeActor>(FirstItem);
			
			if(FirstWeapon)
			{
				// if a weapon, pick up and equip it
				UWeaponBaseComponent* WeaponComponent = FirstWeapon->WeaponMesh;
				if(WeaponComponent)
				{
					if(bIsCarryingWeapon)
					{
						Throw_Server();
					}
					EquippedWeapon->SetWeaponAttribute(WeaponComponent->GetAttribute());
					EquippedWeapon->SetActive(true);
					EquippedWeapon->SetVisibility(true);
					EquippedWeapon->SetWithTarget(true); // use target vector to correct direction
					bIsCarryingWeapon = true;
				}
				FirstItem->Destroy();
			}
			else if(FirstGrenade)
            {
				// if a grenade, pick up and add numbers
                if(CurrentGrenadeNum < MaxGrenadeNum)
                {
                	++CurrentGrenadeNum;
                	
                	FirstItem->Destroy();
                }
            }
			
		}
	}
}

void ANetShootGameCharacter::Throw_Server_Implementation()
{
	UWorld* const World = GetWorld();
	if (bIsCarryingWeapon && World)
	{
		FTransform SpawnTransform = GetMesh()->GetSocketTransform(WeaponSocketName);
		AWeaponBaseActor* const SpawnWeapon = World->SpawnActor<AWeaponBaseActor>(AWeaponBaseActor::StaticClass(), SpawnTransform);
		
		SpawnWeapon->WeaponMesh->SetWeaponAttribute(EquippedWeapon->GetAttribute());

		FVector ThrowImpulse = GetActorForwardVector();
		ThrowImpulse = ThrowImpulse + FVector(0.f, 0.f, 0.2f);
		ThrowImpulse = 800.0f * ThrowImpulse;
		ThrowImpulse = ThrowImpulse + GetMovementComponent()->Velocity;
		SpawnWeapon->WeaponMesh->AddImpulse(ThrowImpulse);
		
		EquippedWeapon->ClearWeaponAttribute();
		EquippedWeapon->SetActive(false);
		EquippedWeapon->SetVisibility(false);
		bIsCarryingWeapon = false;
	}
}

///////////////////////////////////////
/// GamePlay
float ANetShootGameCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser)
{
	const float ActuralDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	if(CurrentHealth > 0)
	{
		if(CurrentHealth <= ActuralDamage)
        {
        	CurrentHealth = .0f;
        	ActorBeKilled(EventInstigator);
        }
        else
        {
        	CurrentHealth -= ActuralDamage;
        	OnCurrentHealthChange_Rep();
        }
	}

	return ActuralDamage;
}

void ANetShootGameCharacter::OnCurrentHealthChange_Rep()
{
	if(GetLifeRatio() <= LowLifeRatio)
 	{
 		LowLifeDelegate.Broadcast();
 	}
}

void ANetShootGameCharacter::ActorBeKilled(AController* KillerController)
{
	if(HasAuthority())
	{
		if(KillerController)
        {
			// killer point + 1 ( about player state, may be a delegate is better? )
        	ANetShootGamePlayerState* KillerPlayerState = KillerController->GetPlayerState<ANetShootGamePlayerState>();
        	if(KillerPlayerState) ++(KillerPlayerState->KillNum);
        }
    
        AController* SelfController = GetController();
        if(SelfController)
        {
        	// be killed actor death + 1
        	ANetShootGamePlayerState* SelfPlayerState = SelfController->GetPlayerState<ANetShootGamePlayerState>();
        	if(SelfPlayerState) ++(SelfPlayerState->DeathNum);
        }

		UE_LOG(LogTemp, Warning, TEXT("Character, before destroy."))
        
	    // Destroy();
		DeathToBeRagDoll_MultiCast();
		RemoveWidgetsOfCharacter();
		GetWorldTimerManager().SetTimer(DestroyTimer, this, &ANetShootGameCharacter::DestroyFunc, 5.0f); 
		DeathDelegate.Broadcast();
		
		UE_LOG(LogTemp, Warning, TEXT("Character, after destroy."))
	}
}

void ANetShootGameCharacter::RespawnSelfClassCharacter_Implementation()
{
	UWorld* World = GetWorld();
	if(World)
	{
		TArray<AActor*> PlayerStarts;
		UGameplayStatics::GetAllActorsOfClass(World, APlayerStart::StaticClass(), PlayerStarts);
		int32 RandomIndex = FMath::RandRange(0, PlayerStarts.Num()-1);
		FTransform RespawnTransform = PlayerStarts[RandomIndex]->GetActorTransform();
		APawn* RespawnPawn = World->SpawnActor<ANetShootGameCharacter>(this->StaticClass(), RespawnTransform);
		GetController()->Possess(RespawnPawn);
	}
}

void ANetShootGameCharacter::DeathToBeRagDoll_MultiCast_Implementation()
{
	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionProfileName(TEXT("Ragdoll"));
	GetMesh()->bDisableClothSimulation = true;
}

/////// IK ///////

float ANetShootGameCharacter::IKFootTrace(FName FootSocketName)
{
	UWorld* World = GetWorld();
	if(World)
	{
		FVector SocketLocation = GetMesh()->GetSocketLocation(FootSocketName);
		FVector ActorLocation = GetActorLocation();
		float HalfLength = GetSimpleCollisionHalfHeight() * GetActorScale().Z; // Transform to World Length
		float TraceLength = 50.0f + HalfLength;
		FVector TraceStart(SocketLocation.X, SocketLocation.Y, ActorLocation.Z);
		FVector TraceEnd(SocketLocation.X, SocketLocation.Y, ActorLocation.Z - TraceLength);

		FHitResult HitResult;
		const FCollisionQueryParams QueryParams(TEXT("IK"), false, this);
		bool HitSuccess = World->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams);

		if(HitSuccess)
		{
			float Distance = ActorLocation.Z - HalfLength - HitResult.Location.Z;

			// Transform to Local Distance
			return Distance / GetActorScale().Z;
		}
	}

	return 0;
}


///// Properties Replicate Config /////

void ANetShootGameCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ANetShootGameCharacter, bIsThrowingGrenade);
	DOREPLIFETIME(ANetShootGameCharacter, bIsCarryingWeapon);
	DOREPLIFETIME(ANetShootGameCharacter, bIsAiming);
	
	DOREPLIFETIME(ANetShootGameCharacter, LookUpAngle);
	
	DOREPLIFETIME(ANetShootGameCharacter, MaxHealth);
	DOREPLIFETIME(ANetShootGameCharacter, CurrentHealth);

	DOREPLIFETIME(ANetShootGameCharacter, CurrentGrenadeNum);
}
