// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "EndMontageNotify.generated.h"

/**
 * 
 */


UCLASS()
class CODEX_API UEndMontageNotify : public UAnimNotify
{
	GENERATED_BODY()

public :
	DECLARE_MULTICAST_DELEGATE(FOnAnimMontageEnd)
	FOnAnimMontageEnd OnAnimMontageEnd;

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation) override;
};
