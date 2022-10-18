// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "USTile.generated.h"

UCLASS()
class UPSKILL_PATHFINDING_API AUSTile : public AActor
{
	GENERATED_BODY()
	
public:	
#pragma region Properties
	UPROPERTY(EditAnywhere, Category = "Core")
		bool showFlowfieldArrowArrow;
#pragma endregion


	// Sets default values for this actor's properties
	AUSTile();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


};
