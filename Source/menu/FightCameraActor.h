// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraActor.h"
#include "Fighter.h"
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
  AFighter* fighter1;
  UPROPERTY(EditAnywhere)
  AFighter* fighter2;

  void BeginPlay();

  void SetFighters(AFighter* fighter1, AFighter* fighter2);

  void Tick(float DeltaTime);
};
