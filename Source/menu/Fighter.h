// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Logic.h"
#include "Fighter.generated.h"

UCLASS()
class MENU_API AFighter : public AActor
{
  GENERATED_BODY()

public:
  // Sets default values for this actor's properties
  AFighter();

private:
  UPROPERTY(EditAnywhere)
  bool isPlayer1;
  ALogic *l;

protected:
  // Called when the game starts or when spawned
  virtual void BeginPlay() override;

public:
  // Called every frame
  virtual void Tick(float DeltaTime) override;

};
