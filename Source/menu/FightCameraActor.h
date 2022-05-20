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
    double scale;
  UPROPERTY(EditAnywhere)
    double min;
  UPROPERTY(EditAnywhere)
    double height;

  void BeginPlay();

  void Tick(float DeltaTime);
};
