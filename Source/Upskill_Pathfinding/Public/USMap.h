// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "USMap.generated.h"

UCLASS()
class UPSKILL_PATHFINDING_API AUSMap : public AActor
{
	GENERATED_BODY()
	
public:	
	
#pragma region Properties
	UPROPERTY(EditAnywhere, Category = "Core")
		TSubclassOf<AActor>Tile;


	UPROPERTY(EditAnywhere, Category = "Map Definition")
		FVector2D MapSize;
	UPROPERTY(EditAnywhere, Category = "Map Definition")
		int TileSize;
#pragma endregion

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Sets default values for this actor's properties
	AUSMap();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

};
