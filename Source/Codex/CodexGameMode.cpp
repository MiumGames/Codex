// Copyright Epic Games, Inc. All Rights Reserved.

#include "CodexGameMode.h"
#include "CodexCharacter.h"
#include "CodexHUD.h"
#include "CodexPlayerController.h"
#include "UObject/ConstructorHelpers.h"

ACodexGameMode::ACodexGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/Blueprints/Player/BP_DefaultCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
	PlayerControllerClass = ACodexPlayerController::StaticClass();
	HUDClass = ACodexHUD::StaticClass();
}
