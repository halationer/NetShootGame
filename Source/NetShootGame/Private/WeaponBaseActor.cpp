// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBaseActor.h"

#include "WeaponBaseComponent.h"
#include "Components/SphereComponent.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"
#include "Serialization/BufferArchive.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"

// Sets default values
AWeaponBaseActor::AWeaponBaseActor()
{
	WeaponMesh = CreateDefaultSubobject<UWeaponBaseComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetCollisionProfileName(TEXT("StaticBlockPhysics"));
	WeaponMesh->SetSimulatePhysics(true);
	WeaponMesh->CanCharacterStepUpOn = ECB_No;

	WeaponMesh->SetIsReplicated(true);
	RootComponent = WeaponMesh;

	AISightSource = CreateDefaultSubobject<UAIPerceptionStimuliSourceComponent>(TEXT("SightSource"));
	// AISightSource->ComponentTags.AddUnique(TEXT("Weapon"));
	
	PickUpSensor->SetupAttachment(RootComponent);

	this->Tags.AddUnique(TEXT("Weapon"));
}

void AWeaponBaseActor::BeginPlay()
{
	Super::BeginPlay();
	// AISightSource->RegisterWithPerceptionSystem();
	AISightSource->RegisterForSense(UAISense_Sight::StaticClass());
}


/////////////////////// IBackpackItemInterface //////////////////////////

bool AWeaponBaseActor::ActorEqualToInfo(AActor* Actor, FBackpackItemInfo& Info)
{
	return IBackpackItemInterface::ActorEqualToInfo(Actor, Info);
}


FBackpackItemInfo AWeaponBaseActor::GenerateBackpackItemInfo_Implementation(AActor* Actor)
{
	// GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5.0f, FColor::Yellow, this->GetClass()->GetName());
	
	FBackpackItemInfo Info = IBackpackItemInterface::GenerateBackpackItemInfo_Implementation(this);
	Info.bIsUnique = true;
	Info.ItemType = FName(WeaponMesh->GetAttribute().Name);
	Info.ItemClass = AWeaponBaseActor::StaticClass();

	FMemoryWriter Writer(Info.ItemStatus, true);
	FObjectAndNameAsStringProxyArchive Ar(Writer, false);
	this->Serialize(Ar);

	return Info;
}


void AWeaponBaseActor::InitItemBeforeSpawn_Implementation(FBackpackItemInfo& Info, AActor* Actor)
{
	IBackpackItemInterface::InitItemBeforeSpawn_Implementation(Info, Actor);
	
	// GEngine->AddOnScreenDebugMessage(INDEX_NONE, 5.0f, FColor::Red, Actor->GetClass()->GetName());

	FMemoryReader Reader(Info.ItemStatus, true);
	FObjectAndNameAsStringProxyArchive Ar(Reader, false);
	AWeaponBaseActor* TempActor = NewObject<AWeaponBaseActor>();
	TempActor->Serialize(Ar);
	
	WeaponMesh->SetWeaponAttribute(TempActor->WeaponMesh->GetAttribute());

    UE_LOG(LogTemp, Warning, TEXT("%s, %d, %s"),
        GetData(WeaponMesh->GetAttribute().Name),
        WeaponMesh->GetAttribute().Id,
        GetData(WeaponMesh->GetAttribute().SkeletonMesh->GetPathName())
    );
}
