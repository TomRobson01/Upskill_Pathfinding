// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "USMap.generated.h"

#define MAX_JPS_JUMP 128

USTRUCT(BlueprintType)
struct FNodeRecord
{
	GENERATED_BODY()

	AActor* node;
	AActor* fromNode;
	float costSoFar;
	float estimatedTotalCost;
};

UENUM(BlueprintType)
enum EPathfindingMethod
{
	PATHFIND_None UMETA(DisplayName = "None"),
	PATHFIND_AStar UMETA(DisplayName = "A*"),
	PATHFIND_Jump UMETA(DisplayName = "Jump Point Search"),
	PATHFIND_Flow UMETA(DisplayName = "Flowfields")
};

UCLASS()
class UPSKILL_PATHFINDING_API AUSMap : public AActor
{
	GENERATED_BODY()
	
public:	
	
#pragma region Properties
	UPROPERTY(EditAnywhere, Category = "Core")
		TSubclassOf<AActor>Tile;

	UPROPERTY(EditAnywhere, Category = "Path")
		TEnumAsByte<EPathfindingMethod> pathingMethod;
	UPROPERTY(EditAnywhere, Category = "Path")
		FVector2D destinationNoiseRanges;

	UPROPERTY(EditAnywhere, Category = "Map Definition")
		FString MapFilePath;
	UPROPERTY(EditAnywhere, Category = "Map Definition")
		FVector2D MapSize;
	UPROPERTY(EditAnywhere, Category = "Map Definition")
		int TileSize;
#pragma endregion

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	TArray<FString> ReadStringFromFile(FString FilePath);

public:	

#pragma region Blueprint Callable Functions
	// DEBUG: Functions callable from BPs to handle debugging at runtime
	UFUNCTION(BlueprintCallable, Category = "US Map - Debug")
		void SetPathfindingMethod(EPathfindingMethod eMethod) { pathingMethod = eMethod; };

	UFUNCTION(BlueprintCallable, Category = "US Map - Debug")
		void Debug_TogglePathlines();
	UFUNCTION(BlueprintCallable, Category = "US Map - Debug")
		void Debug_ToggleAStarCosts() { ShowAStarCosts = !ShowAStarCosts; };
	UFUNCTION(BlueprintCallable, Category = "US Map - Debug")
		void Debug_ToggleAStarNodes() { ShowAStarNodes = !ShowAStarNodes; };
	UFUNCTION(BlueprintCallable, Category = "US Map - Debug")
		void Debug_ToggleJPSNodes() { ShowJPSNodes = !ShowJPSNodes; };
	UFUNCTION(BlueprintCallable, Category = "US Map - Debug")
		void Debug_ToggleFlowField() { ShowFlowField = !ShowFlowField; };
	UFUNCTION(BlueprintCallable, Category = "US Map - Debug")
		void Debug_ToggleIntegrationField() { ShowIntegrationField = !ShowIntegrationField;  };
	UFUNCTION(BlueprintCallable, Category = "US Map - Debug")
		void Debug_ToggleCostField() { ShowCostField = !ShowCostField; };
#pragma endregion

	// Sets default values for this actor's properties
	AUSMap();

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void RegisterAgent(AActor* aAgent);
	void StartPathGen(AActor* aPathTarget);

	void AStar_ProcessNode(AActor* aNode, AActor* aFromNode, float fCostSoFar, float fEstimatedCost, TArray<FNodeRecord>* aNodeToAddTo);
	void JPS_InitiateJump(FNodeRecord bestNode, TArray<FNodeRecord>* aOpenList, TArray<FNodeRecord>* aClosedList, AActor* aPathTarget);
	void Flowfield_GenerateIntegrationField(AActor* aPathTarget);

private:
	TArray<TArray<AActor*>> tiles;
	TArray<AActor*> agents;

	// DEBUG: Bools enabling/disabling debug settings
	bool ShowAStarCosts;
	bool ShowAStarNodes;
	bool ShowJPSNodes;
	bool ShowFlowField;
	bool ShowIntegrationField;
	bool ShowCostField;

};
