// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraActor.h"
#include "FightCameraActor.generated.h"

/**
 *
 */
UCLASS()
class MENU_API AFightCameraActor : public ACameraActor
{
  GENERATED_BODY()

public:
  AFightCameraActor();

  UPROPERTY(EditAnywhere)
  AActor* fighter1;
  UPROPERTY(EditAnywhere)
  AActor* fighter2;

  UPROPERTY(EditAnywhere)
  float stageBoundLeft;
  UPROPERTY(EditAnywhere)
  float stageBoundRight;

  // these get reset to their true values in BeginPlay but are useful
  // for experimenting with new values
  UPROPERTY(EditAnywhere)
    double scale;
  UPROPERTY(EditAnywhere)
    double min;
  UPROPERTY(EditAnywhere)
    double height;
  UPROPERTY(EditAnywhere)
    double boundAdjust;

  void BeginPlay();

  void Tick(float DeltaTime);
};
