// Copyright Epic Games, Inc. All Rights Reserved.


#include "Upskill_PathfindingGameModeBase.h"

void AUpskill_PathfindingGameModeBase::BeginPlay()
{
	Super::BeginPlay();

	// Spawn the USMap core, and then run any setup functions needed
	GetWorld()->SpawnActor(MapCore);
}