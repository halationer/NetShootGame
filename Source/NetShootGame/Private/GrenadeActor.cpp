// Fill out your copyright notice in the Description page of Project Settings.


#include "GrenadeActor.h"

#include "GrenadeComponent.h"
#include "Components/SphereComponent.h"

AGrenadeActor::AGrenadeActor()
{
	GrenadeMesh = CreateDefaultSubobject<UGrenadeComponent>(TEXT("GrenadeMesh"));
	GrenadeMesh->SetCollisionProfileName(TEXT("StaticBlockPhysics"));
	GrenadeMesh->SetSimulatePhysics(true);
	GrenadeMesh->SetLinearDamping(100.0f);
	GrenadeMesh->SetAngularDamping(80.0f);
	GrenadeMesh->CanCharacterStepUpOn = ECB_No;
	GrenadeMesh->SetIsReplicated(true);
	RootComponent = GrenadeMesh;

	PickUpSensor->SetupAttachment(RootComponent);

	GrenadeMesh->ExplodeFinishDelegate.AddDynamic(this, &AGrenadeActor::OnExplodeFinish);
}

void AGrenadeActor::OnExplodeFinish()
{
	Destroy();
}