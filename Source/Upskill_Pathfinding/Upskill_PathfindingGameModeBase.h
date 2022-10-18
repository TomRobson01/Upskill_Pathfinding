// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "Upskill_PathfindingGameModeBase.generated.h"

/**
 * 
 */
UCLASS()
class UPSKILL_PATHFINDING_API AUpskill_PathfindingGameModeBase : public AGameModeBase
{
	GENERATED_BODY()
public: 
	virtual void BeginPlay() override;
	
#pragma region Properties
	UPROPERTY(EditAnywhere, Category = "Core")
		TSubclassOf<AActor>MapCore;
#pragma endregion
};
