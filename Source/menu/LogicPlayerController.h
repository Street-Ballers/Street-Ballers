// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Input.h"
#include "LogicPlayerController.generated.h"

UCLASS()
class MENU_API ALogicPlayerController : public APlayerController
{
  GENERATED_BODY()

private:
  int playerNumber;
  Input* input;

public:
  ALogicPlayerController();

  void PostLogin(int playerNumber_);

  void Tick(float deltaSeconds);
};
