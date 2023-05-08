// Fill out your copyright notice in the Description page of Project Settings.


#include "EndMontageNotify.h"


void UEndMontageNotify::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
	Super::Notify(MeshComp, Animation);

	OnAnimMontageEnd.Broadcast();
}
