// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "USMap.h"
#include "Kismet/KismetMathLibrary.h"
#include "USAgent.generated.h"

UCLASS()
class UPSKILL_PATHFINDING_API AUSAgent : public AActor
{
	GENERATED_BODY()
	
public:	

#pragma region Properties
	UPROPERTY(EditAnywhere, Category = "US Agent Controls")
		float moveSpeed = 5;
	UPROPERTY(EditAnywhere, Category = "US Agent Controls")
		float stopDistance = 1;

	UPROPERTY(EditAnywhere, Category = "Marker")
		TSubclassOf<AActor>PathMarker;
	UPROPERTY(EditAnywhere, Category = "Marker")
		float debugLineHeight = 1;
#pragma endregion

	// Sets default values for this actor's properties
	AUSAgent();

	void SetupAgent(AUSMap* aMap, AActor* aStartTile);
	void NavigatePath(float fDeltaTime);
	void SetPath(TArray<FVector2D> vPath, AActor* aDestinationTile) { path = vPath; currentPathIndex = 0; currentTile = aDestinationTile; DrawPathViz(); usingFlowField = false;}
	void SetFlowFieldPath(AActor* aDestinationTile) { ffDestinationTile = aDestinationTile; usingFlowField = true; }

	void ToggleShowPathLine() { ShowPathLine = !ShowPathLine; };

	AActor* currentTile;
	AActor* ffDestinationTile;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	AUSMap* map;

	void FaceLocation(FVector2D vLookLocation);
	void DrawPathViz();

private:
	TArray<FVector2D> path;
	TArray<AActor*> pathMarkers; 
	int currentPathIndex;
	bool usingFlowField;

	bool ShowPathLine;
};
