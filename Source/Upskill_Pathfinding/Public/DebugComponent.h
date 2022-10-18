// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DebugComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class UPSKILL_PATHFINDING_API UDebugComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
#pragma region Properties


#pragma endregion

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Sets default values for this component's properties
	UDebugComponent();

	UFUNCTION(BlueprintCallable, Category = "Debug Component")
	void SendDebugMessage(FString messageText, float messageDuration = 2.f, FColor messageColor = FColor::Yellow);
};
